#include "utils.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <re2/re2.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wincrypt.h>
#else
#include <openssl/evp.h>
#endif

#include "common/log.h"
#include "common/hash.h"
#include "common/types.h"

static const std::pair<RE2, re2::StringPiece> path_replace_pattern[]
{
    {R"(\\)", R"(\\\\)"},
    {R"(\.)", R"(\\.)"},
    {R"(\^)", R"(\\^)"},
    {R"(\$)", R"(\\$)"},
    {R"(\|)", R"(\\|)"},
    {R"(\()", R"(\\()"},
    {R"(\))", R"(\\))"},
    {R"(\{)", R"(\\{)"},
    {R"(\{)", R"(\\{)"},
    {R"(\[)", R"(\\[)"},
    {R"(\])", R"(\\])"},
    {R"(\+)", R"(\\+)"},
    {R"(\/)", R"(\\/)"},
    {R"(\?)", R"([^\\\\])"},
    {R"(\*)", R"([^\\\\]*)"},
};

std::vector<Path> findFiles(Path p, bool recursive)
{
	auto pstr = p.make_preferred().native();
	size_t offset = pstr.find('*');

	std::vector<Path> res;
	if (offset == pstr.npos)
	{
		if (!pstr.empty())
			res.push_back(std::move(p));
		return res;
	}

    StringPath folder = pstr.substr(0, offset).substr(0, pstr.find_last_of(Path::preferred_separator));
    if (fs::is_directory(folder))
    {
        pstr = pstr.substr(pstr[folder.length() - 1] == Path::preferred_separator ? folder.length() : folder.length() + 1);

        std::string str = Path(pstr).u8string();
        for (const auto& [in, out] : path_replace_pattern)
        {
            RE2::GlobalReplace(&str, in, out);
        }

        auto pathRegex = RE2(str);
        Path pathFolder(folder);
        for (auto& f : fs::recursive_directory_iterator(pathFolder))
        {
            if (recursive)
            {
                if (f.path().filename().u8string().substr(0, 2) != u8"._" && RE2::FullMatch(f.path().filename().u8string(), pathRegex))
                {
                    res.push_back(f.path());
                }
            }
            else
            {
                auto relativeFilePath = fs::relative(f, pathFolder);
                if (relativeFilePath.u8string().substr(0, 2) != u8"._" && RE2::FullMatch(relativeFilePath.u8string(), pathRegex))
                {
                    res.push_back(f.path());
                }
            }
        }
    }

	return res;
}

bool isParentPath(Path parent, Path dir)
{
    parent = fs::absolute(parent);
    dir = fs::absolute(dir);

    if (parent.root_directory() != dir.root_directory())
        return false;

    auto pair = std::mismatch(dir.begin(), dir.end(), parent.begin(), parent.end());
    return pair.second == parent.end();
}

int toInt(std::string_view str, int defVal) noexcept
{
    int val = 0;
    if (auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), val); ec == std::errc())
        return val;
    return defVal;
}

double toDouble(std::string_view str, double defVal) noexcept
{
    double val = 0;
    if (auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), val); ec == std::errc())
        return val;
    return defVal;
}

bool lunaticvibes::iequals(std::string_view lhs, std::string_view rhs) noexcept
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        [](unsigned char l, unsigned char r) { return std::tolower(l) == std::tolower(r); });
}

// TODO(C++20): use std::span.
std::string bin2hex(const void* bin, size_t size)
{
    std::string res;
    res.reserve(size * 2 + 1);
    constexpr char rgbDigits[] = "0123456789abcdef";
    for (size_t i = 0; i < size; ++i)
    {
        unsigned char c = ((unsigned char*)bin)[i];
        res += rgbDigits[(c >> 4) & 0xf];
        res += rgbDigits[(c >> 0) & 0xf];
    }
    return res;
}

std::string hex2bin(std::string_view hex)
{
    std::string res;
    res.resize(hex.length() / 2 + 1);
    for (size_t i = 0, j = 0; i < hex.length() - 1; i += 2, j++)
    {
        unsigned char &c = ((unsigned char&)res[j]);
        unsigned char c1 = tolower(hex[i]);
        unsigned char c2 = tolower(hex[i + 1]);
        c += (c1 + ((c1 >= 'a') ? (10 - 'a') : (-'0'))) << 4;
        c += (c2 + ((c2 >= 'a') ? (10 - 'a') : (-'0')));
    }
    res[res.length() - 1] = '\0';
    return res;
}

#ifdef _WIN32

HashMD5 md5(std::string_view s)
{
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;

    CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
    CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);

    CryptHashData(hHash, (const BYTE*)s.data(), s.size(), 0);

    constexpr size_t MD5_LEN = 16;
    BYTE rgbHash[MD5_LEN];
    DWORD cbHash = MD5_LEN;
    CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0);

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    return bin2hex(rgbHash, MD5_LEN);
}

HashMD5 md5file(const Path& filePath)
{
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath))
    {
        return {};
    }

    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;

    CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
    CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);

    std::ifstream ifs(filePath, std::ios::in | std::ios::binary);
    while (!ifs.eof())
    {
        constexpr size_t BUFSIZE = 1024;
        char rgbFile[BUFSIZE];
        DWORD cbRead = 0;
        ifs.read(rgbFile, BUFSIZE);
        cbRead = ifs.gcount();
        if (cbRead == 0) break;
        CryptHashData(hHash, (const BYTE*)rgbFile, cbRead, 0);
    }

    constexpr size_t MD5_LEN = 16;
    BYTE rgbHash[MD5_LEN];
    DWORD cbHash = MD5_LEN;
    CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0);

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    return bin2hex(rgbHash, MD5_LEN);
}

#else

// TODO: std::span
static HashMD5 format_hash(const unsigned char* digest, size_t digest_size)
{
    std::string ret;
    for (size_t i = 0; i < digest_size; ++i)
    {
        unsigned char high = digest[i] >> 4 & 0xF;
        unsigned char low  = digest[i] & 0xF;
        ret += (high <= 9 ? ('0' + high) : ('A' - 10 + high));
        ret += (low  <= 9 ? ('0' + low)  : ('A' - 10 + low));
    }
    return ret;
}

template <typename DigestUpdater> HashMD5 md5_impl(DigestUpdater updater)
{
    struct MdCtxDeleter
    {
        void operator()(EVP_MD_CTX *ctx)
        {
            EVP_MD_CTX_free(ctx);
        }
    };
    auto ctx = std::unique_ptr<EVP_MD_CTX, MdCtxDeleter>(EVP_MD_CTX_new());
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    if (!EVP_DigestInit_ex2(ctx.get(), EVP_md5(), NULL))
    {
        LOG_ERROR << "EVP_DigestInit_ex2() failed";
        return {};
    };
    if (!updater(ctx.get()))
    {
        LOG_ERROR << "digest updating failed";
        return {};
    }
    if (!EVP_DigestFinal_ex(ctx.get(), digest, &digest_len))
    {
        LOG_ERROR << "EVP_DigestFinal_ex() failed";
        return {};
    };

    return format_hash(digest, digest_len);
}

// TODO(C++20): take std::span instead
HashMD5 md5(std::string_view s)
{
    return md5_impl([&](EVP_MD_CTX *context) { return EVP_DigestUpdate(context, s.data(), s.size()) != 0; });
}

HashMD5 md5file(const Path &filePath)
{
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath))
    {
        LOG_WARNING << "path is not a file or does not exist";
        return {};
    }
    std::ifstream ifs{filePath, std::ios::binary};
    return md5_impl([&](EVP_MD_CTX *context) {
        std::array<char, 1024> buf;
        while (!ifs.eof())
        {
            ifs.read(buf.data(), buf.size());
            if (!EVP_DigestUpdate(context, buf.data(), ifs.gcount()))
                return false;
        }
        return true;
    });
}

#endif

std::string toLower(std::string_view s)
{
	std::string ret(s);
	for (auto& c : ret)
		if (c >= 'A' && c <= 'Z')
			c = c - 'A' + 'a';
	return ret;
}

std::string toUpper(std::string_view s)
{
	std::string ret(s);
	for (auto& c : ret)
		if (c >= 'a' && c <= 'z')
			c = c - 'a' + 'A';
	return ret;
}

#ifndef _WIN32

// Resolve case-insensitive path on case-sensitive file systems.
// This does IO.
// Preserves non-existing paths. Does not trim redundant `./`.
// Length out of the returned string is the same as of the input string.
//
// This function assumes paths without drive letter assignments and
// operates on strings directly.
// In addition, directories must be separated by forward slashes.
//
// Example:
// On your system there is a `/tmp/AFileWithUpperCaseInIt`.
// `resolvePathCaseInsensitively("/tmp/afilewithuppercaseinit")` =
// `"/tmp/AFileWithUpperCaseInIt"`
static std::string resolveCaseInsensitivePath(std::string input)
{
    if (input == "/" || input == "." || input.empty()) {
        return input;
    }

    static constexpr std::string_view CURRENT_PATH_RELATIVE_PREFIX = "./";
    const char first_character = input[0];
    // Used to determine if this is a relative path without a leading
    // `./`. If so, we prepend it to the string temporarily, so that we
    // can use `filesystem::directory_iterator`. After we are done, we
    // will remove this prepended prefix.
    const bool has_path_prefix = (first_character == '.') || (first_character == '/');

    std::string out;
    out.reserve(input.length() + CURRENT_PATH_RELATIVE_PREFIX.length());

    std::vector<std::string> segments;
    boost::split(segments, input, boost::is_any_of("/"));

    size_t segments_traversed = 0;
    for (const auto& segment : segments) {
        if (segment.empty()) {
            segments_traversed += 1;
            continue;
        }

        std::string_view prefix;
        if (segment == ".") {
            prefix = CURRENT_PATH_RELATIVE_PREFIX;
        } else if (segment == "..") {
            prefix = "../";
        }
        if (!prefix.empty()) {
            if (!out.empty() && out.back() != '/') {
                out += '/';
            }
            out += prefix;
            segments_traversed += 1;
            continue;
        }

        const bool is_empty = out.empty();
        if (is_empty && !has_path_prefix) {
            out = CURRENT_PATH_RELATIVE_PREFIX;
        } else if (is_empty || out.back() != '/') {
            out += '/';
        }

        bool found_entry = false;

        for (const auto& dir_entry : std::filesystem::directory_iterator(out)) {
            const auto dir_entry_name = dir_entry.path().filename().u8string();
            if (lunaticvibes::iequals(dir_entry_name, segment)) {
                found_entry = true;
                out += dir_entry_name;
                break;
            }
        }

        segments_traversed += 1;
        if (!found_entry) {
            out += segment;
            break;
        }
    }

    for (; segments_traversed < segments.size(); ++segments_traversed) {
        out += '/';
        out += segments[segments_traversed];
    }

    if (!has_path_prefix) {
        out.erase(0, CURRENT_PATH_RELATIVE_PREFIX.length());
    }

    return out;
}

#endif // _WIN32

std::string convertLR2Path(const std::string& lr2path, const Path& relative_path)
{
    return convertLR2Path(lr2path, relative_path.u8string());
}

std::string convertLR2Path(const std::string& lr2path, const std::string& relative_path_utf8)
{
    return convertLR2Path(lr2path, std::string_view(relative_path_utf8));
}

std::string convertLR2Path(const std::string& lr2path, const char* relative_path_utf8)
{
    return convertLR2Path(lr2path, std::string_view(relative_path_utf8));
}

std::string convertLR2Path(const std::string& lr2path, std::string_view relative_path_utf8)
{
    std::string_view raw = lunaticvibes::trim(relative_path_utf8, "\"");
    if (raw.empty())
        return {};
    if (auto p = PathFromUTF8(raw); p.is_absolute())
    {
        LOG_WARNING << "absolute path in an LR2 path, this is forbidden, raw=" << raw;
        return {};
    }

    std::string_view prefix = raw.substr(0, 2);
    if (prefix == "./" || prefix == ".\\")
    {
        raw = raw.substr(2);
    }
    else if (prefix[0] == '/' || prefix[0] == '\\')
    {
        raw = raw.substr(1);
    }
    std::string out_path;
    if (lunaticvibes::iequals(raw.substr(0, 8), "LR2files"))
    {
        Path path = PathFromUTF8(lr2path);
        path /= PathFromUTF8(raw);
        out_path = path.u8string();
    }
    else
    {
        out_path = std::string(raw);
    }

#ifndef _WIN32
    std::replace(out_path.begin(), out_path.end(), '\\', '/');
    out_path = resolveCaseInsensitivePath(out_path);
#endif // _WIN32

    return out_path;
}

Path PathFromUTF8(std::string_view s)
{
    return fs::u8path(s);
}

void preciseSleep(long long sleep_ns)
{
    if (sleep_ns <= 0) return;

    using namespace std::chrono;
    using namespace std::chrono_literals;

#ifdef _WIN32

    static HANDLE timer = CreateWaitableTimer(NULL, FALSE, NULL);

    while (sleep_ns > 1'000'000)
    {
        LARGE_INTEGER due;
        due.QuadPart = -int64_t((sleep_ns - sleep_ns % 1'000'000) / 100);  // wrap to 1ms

        auto start = high_resolution_clock::now();
        SetWaitableTimerEx(timer, &due, 0, NULL, NULL, NULL, 0);
        WaitForSingleObjectEx(timer, INFINITE, TRUE);
        auto end = high_resolution_clock::now();

        double observed = duration_cast<nanoseconds>(end - start).count();
        sleep_ns -= observed;
    }

    // spin lock
    auto start = high_resolution_clock::now();
    while (duration_cast<nanoseconds>(high_resolution_clock::now() - start).count() < sleep_ns);

#else
    const auto duration = std::chrono::nanoseconds(sleep_ns);

    // FIXME not optimized and not accurate
    std::this_thread::sleep_for(duration);

#endif
}

double normalizeLinearGrowth(double prev, double curr)
{
    if (prev == -1.0) return 0.0;
    if (curr == -1.0) return 0.0;

    assert(prev >= 0.0 && prev <= 1.0);
    assert(curr >= 0.0 && curr <= 1.0);

    double delta = curr - prev;
    if (prev > 0.8 && curr < 0.2)
        delta += 1.0;
    else if (prev < 0.2 && curr > 0.8)
        delta -= 1.0;

    assert(delta >= -1.0 && delta <= 1.0);
    return delta;
}

void lunaticvibes::trim_in_place(std::string& s) {
    static auto not_space = [](int c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
}
