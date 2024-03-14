#pragma once

#include <cstring>
#include <functional>
#include <string>
#include <string_view>

#include "utils.h"

template <size_t _Len>
class Hash
{
private:
    unsigned char data[_Len] = { 0 };
    bool set = false;
public:
    Hash() = default;
    Hash(const std::string& hex)
    {
        reset();
        if (!hex.empty())
        {
            assert(hex.size() == _Len * 2);
            set = true;
            std::string bin = hex2bin(hex);
            unsigned char* ubin = (unsigned char*)bin.data();
            std::memcpy(data, ubin, _Len);
        }
    }
    Hash(const Hash<_Len>& rhs)
    {
        reset();
        if (!rhs.empty())
        {
            set = true;
            std::memcpy(data, rhs.data, _Len);
        }
    }

    constexpr size_t length() const { return _Len; }
    bool empty() const { return !set; }
    std::string hexdigest() const { return bin2hex(data, _Len); }
    const unsigned char* hex() const { return data; }
    void reset() { set = false; memset(data, 0, _Len); }

    template <size_t _Len2>
    bool operator<(const Hash<_Len2>& rhs) const { static_assert(_Len == _Len2); return memcmp(data, rhs.data, _Len) < 0; }
    template <size_t _Len2>
    bool operator>(const Hash<_Len2>& rhs) const { static_assert(_Len == _Len2); return memcmp(data, rhs.data, _Len) > 0; }
    template <size_t _Len2>
    bool operator<=(const Hash<_Len2>& rhs) const { static_assert(_Len == _Len2); return !(*this > rhs); }
    template <size_t _Len2>
    bool operator>=(const Hash<_Len2>& rhs) const { static_assert(_Len == _Len2); return !(*this > rhs); }
    template <size_t _Len2>
    bool operator==(const Hash<_Len2>& rhs) const { return _Len == _Len2 && memcmp(data, rhs.data, _Len) == 0; }
    template <size_t _Len2>
    bool operator!=(const Hash<_Len2>& rhs) const { return _Len != _Len2 || memcmp(data, rhs.data, _Len) != 0; }

    friend struct std::hash<Hash<_Len>>;
};

template<size_t _Len>
struct std::hash<Hash<_Len>>
{
    size_t operator()(const Hash<_Len>& obj) const
    {
        return std::hash<std::string_view>()({reinterpret_cast<const char*>(obj.data), _Len});
    }
};

typedef Hash<16> HashMD5;
typedef Hash<32> HashSHA1;

HashMD5 md5(std::string_view str);
HashMD5 md5file(const Path& filePath);
