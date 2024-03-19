#pragma once
#include <any>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <exception>

#include <common/types.h>

struct sqlite3;

#if defined(_MSC_VER)
  typedef __int64 sqlite_int64;
  typedef unsigned __int64 sqlite_uint64;
#else
  typedef long long int sqlite_int64;
  typedef unsigned long long int sqlite_uint64;
#endif
typedef sqlite_int64 sqlite3_int64;
typedef sqlite_uint64 sqlite3_uint64;

inline long long    ANY_INT(const std::any& a) { return std::any_cast<sqlite3_int64>(a); }
inline double       ANY_REAL(const std::any& a) { return std::any_cast<double>(a); }
inline std::string  ANY_STR(const std::any& a) { return std::any_cast<std::string>(a); }


#define SQLITE_OK 0

class SQLite
{
private:
    mutable sqlite3* _db = NULL;
    mutable std::string _lastSql;
    std::string tag;
    bool inTransaction = false;
public:
    SQLite() = delete;
    SQLite(const char* path, std::string tag);
    SQLite(const Path &path, std::string tag) : SQLite(path.u8string().c_str(), std::move(tag))
    {
    }
    virtual ~SQLite();

protected:
    [[nodiscard]] std::vector<std::vector<std::any>> query(std::string_view stmt,
                                                           std::initializer_list<std::any> args = {}) const;
    int exec(std::string_view stmt, std::initializer_list<std::any> args = {});
    void commit();

public:
    void transactionStart();
    void transactionStop();
    void optimize();
    const char* errmsg() const;
};
