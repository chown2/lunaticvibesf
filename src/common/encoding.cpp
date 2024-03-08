#include "encoding.h"

#include <fstream>
#include <istream>
#include <memory>
#include <string>
#include <string_view>

#include <uchardet.h>

#include "common/log.h"
#include "common/sysutil.h"

eFileEncoding getFileEncoding(const Path& path)
{
    std::ifstream fs(path);
    if (fs.fail())
    {
        return eFileEncoding::LATIN1;
    }
    return getFileEncoding(fs);
}

struct UchardetDeleter {
    void operator()(uchardet_t det)
    {
        uchardet_delete(det);
    }
};
using UchardetPtr = std::unique_ptr<std::remove_pointer<uchardet_t>::type, UchardetDeleter>;

eFileEncoding getFileEncoding(std::istream& is)
{
    std::streampos oldPos = is.tellg();

    is.clear();
    is.seekg(0);

    UchardetPtr det{uchardet_new()};
    if (det == nullptr)
    {
        LOG_ERROR << "uchardet_new() error";
        return eFileEncoding::LATIN1;
    }
    for (std::string buf; std::getline(is, buf);)
    {
        if (uchardet_handle_data(det.get(), buf.data(), buf.size()) != 0)
        {
            LOG_ERROR << "uchardet_handle_data() error";
            break;
        }
    }
    uchardet_data_end(det.get());

    is.clear();
    is.seekg(oldPos);

    const std::string_view charset = uchardet_get_charset(det.get());
    if (charset.empty())
    {
        LOG_ERROR << "uchardet_get_charset() error";
        return eFileEncoding::LATIN1;
    }
    if (charset == "ASCII")
        return eFileEncoding::LATIN1;
    if (charset == "SHIFT_JIS")
        return eFileEncoding::SHIFT_JIS;
    if (charset == "EUC_KR")
    {
        LOG_WARNING << "EUC_KR file encoding detected; this messages is here to gauge how often it appears in BMS";
        return eFileEncoding::EUC_KR;
    }
    if (charset == "UTF-8")
        return eFileEncoding::UTF8;
    LOG_ERROR << "file has an unsupported charset: " << charset;
    return eFileEncoding::LATIN1;
}

const char* getFileEncodingName(eFileEncoding enc)
{
    switch (enc)
    {
    case eFileEncoding::EUC_KR: 
        return "EUC-KR";
    case eFileEncoding::LATIN1: 
        return "Latin 1";
    case eFileEncoding::SHIFT_JIS: 
        return "Shift JIS";
    case eFileEncoding::UTF8: 
        return "UTF-8";
    default:
        return "Unknown";
    }
}


#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::string to_utf8(const std::string& input, eFileEncoding fromEncoding)
{
    int cp = CP_UTF8;
    switch (fromEncoding)
    {
    case eFileEncoding::SHIFT_JIS:  cp = 932; break;
    case eFileEncoding::EUC_KR:     cp = 949; break;
    case eFileEncoding::LATIN1:     cp = CP_ACP; break;
    default:                        cp = CP_UTF8; break;
    }
    if (cp == CP_UTF8) return input;

    DWORD dwNum;

    dwNum = MultiByteToWideChar(cp, 0, input.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[dwNum];
    MultiByteToWideChar(cp, 0, input.c_str(), -1, wstr, dwNum);

    dwNum = WideCharToMultiByte(CP_UTF8, NULL, wstr, -1, NULL, 0, NULL, FALSE);
    char* ustr = new char[dwNum];
    WideCharToMultiByte(CP_UTF8, NULL, wstr, -1, ustr, dwNum, NULL, FALSE);

    std::string ret(ustr);

    delete[] wstr;
    delete[] ustr;
    return ret;
}


std::string from_utf8(const std::string& input, eFileEncoding toEncoding)
{
    int cp = CP_UTF8;
    switch (toEncoding)
    {
    case eFileEncoding::SHIFT_JIS:  cp = 932; break;
    case eFileEncoding::EUC_KR:     cp = 949; break;
    case eFileEncoding::LATIN1:     cp = CP_ACP; break;
    default:                        cp = CP_UTF8; break;
    }
    if (cp == CP_UTF8) return input;

    DWORD dwNum;

    dwNum = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[dwNum];
    MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, wstr, dwNum);

    dwNum = WideCharToMultiByte(cp, NULL, wstr, -1, NULL, 0, NULL, FALSE);
    char* lstr = new char[dwNum];
    WideCharToMultiByte(cp, NULL, wstr, -1, lstr, dwNum, NULL, FALSE);

    std::string ret(lstr);

    delete[] wstr;
    delete[] lstr;
    return ret;
}

#else

#include <cerrno>
#include <cstring>
#include <memory>
#include <type_traits>
#include <vector>

#include <iconv.h>

static const char* get_iconv_encoding_name(eFileEncoding encoding)
{
    switch (encoding) {
    case eFileEncoding::LATIN1:
        return "ISO-8859-1";
    case eFileEncoding::SHIFT_JIS:
        return "CP932";
    case eFileEncoding::EUC_KR:
        return "CP949";
    case eFileEncoding::UTF8:
        return "UTF-8";
    }
    panic("Error", "Incorrect eFileEncoding");
    return "INVALID-DUMMY";
}

struct IcdDeleter {
    void operator()(iconv_t icd) {
        int ret = iconv_close(icd);
        if (ret == -1) {
            const int error = errno;
            LOG_ERROR << "iconv_close() error: " << safe_strerror(error) << " (" << error << ")";
        }
    }
};
using IcdPtr = std::unique_ptr<std::remove_pointer<iconv_t>::type, IcdDeleter>;

static std::string convert(const std::string& input, eFileEncoding from, eFileEncoding to)
{
    const auto* source_encoding_name = get_iconv_encoding_name(from);
    const auto* target_encoding_name = get_iconv_encoding_name(to);

    auto icd = IcdPtr(iconv_open(target_encoding_name, source_encoding_name));
    if (reinterpret_cast<size_t>(icd.get()) == static_cast<size_t>(-1)) {
        const int error = errno;
        LOG_ERROR << "iconv_open() error: " << safe_strerror(error) << " (" << error << ")";
        return "(conversion descriptor opening error)";
    }

    static constexpr size_t BUF_SIZE = 1024l * 32l;
    // I wanted to avoid manually allocating here so that we don't have
    // to clean up manually in all return paths.
    char out_buf[BUF_SIZE] = { 0 };

    // BRUH-cast.
    char* buf_ptr = const_cast<char*>(input.c_str());
    std::size_t buf_len = input.length();
    char* out_ptr = static_cast<char*>(out_buf);
    std::size_t out_len = sizeof(out_buf);

    std::size_t iconv_ret = iconv(icd.get(), &buf_ptr, &buf_len, &out_ptr, &out_len);
    if (iconv_ret == static_cast<size_t>(-1)) {
        const int error = errno;
        LOG_ERROR << "iconv() error: " << safe_strerror(error) << " (" << error << ")";
        return "(conversion error)";
    }

    return std::string{static_cast<char*>(out_buf)};
}

std::string to_utf8(const std::string& input, eFileEncoding fromEncoding)
{
    return convert(input, fromEncoding, eFileEncoding::UTF8);
}

std::string from_utf8(const std::string& input, eFileEncoding toEncoding)
{
    return convert(input, eFileEncoding::UTF8, toEncoding);
}

#endif // _WIN32

std::u32string to_utf32(const std::string& input, eFileEncoding fromEncoding)
{
    std::string inputUTF8 = to_utf8(input, fromEncoding);
    return utf8_to_utf32(inputUTF8);
}

std::string from_utf32(const std::u32string& input, eFileEncoding toEncoding)
{
    std::string inputUTF8 = utf32_to_utf8(input);
    return from_utf8(inputUTF8, toEncoding);
}


std::u32string utf8_to_utf32(const std::string& str)
{
    static const auto locale = std::locale("");
    static const auto& facet_u32_u8 = std::use_facet<std::codecvt<char32_t, char, std::mbstate_t>>(locale);
    std::u32string u32Text(str.size() * facet_u32_u8.max_length(), '\0');

    std::mbstate_t s;
    const char* from_next = &str[0];
    char32_t* to_next = &u32Text[0];

    std::codecvt_base::result res;
    do {
        res = facet_u32_u8.in(s,
            from_next, &str[str.size()], from_next,
            to_next, &u32Text[u32Text.size()], to_next);

        // skip unconvertiable chars (which is impossible though)
        if (res == std::codecvt_base::error)
            from_next++;

    } while (res == std::codecvt_base::error);

    u32Text.resize(to_next - &u32Text[0]);
    return u32Text;
}

std::string utf32_to_utf8(const std::u32string& str)
{
    static const auto locale = std::locale("");
    static const auto& facet_u32_u8 = std::use_facet<std::codecvt<char32_t, char, std::mbstate_t>>(locale);
    std::string u8Text(str.size() * 4, '\0');

    std::mbstate_t s;
    const char32_t* from_next = &str[0];
    char* to_next = &u8Text[0];

    std::codecvt_base::result res;
    do {
        res = facet_u32_u8.out(s,
            from_next, &str[str.size()], from_next,
            to_next, &u8Text[u8Text.size()], to_next);

        // skip unconvertiable chars (which is impossible though)
        if (res == std::codecvt_base::error)
            from_next++;

    } while (res == std::codecvt_base::error);

    u8Text.resize(to_next - &u8Text[0]);
    return u8Text;
}