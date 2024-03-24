#ifdef __linux__

#include "sysutil.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <thread>

#include <boost/format.hpp>

#include <sys/prctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <common/utils.h>
#include <common/log.h>

static std::thread::id s_main_thread {};

void SetThreadAsMainThread()
{
    s_main_thread = std::this_thread::get_id();
}

int64_t GetCurrentThreadID()
{
    return static_cast<int64_t>(gettid());
}

bool IsMainThread()
{
    return s_main_thread == std::this_thread::get_id();
}

void SetThreadName(const char* name)
{
    // > The  name  can  be up to 16 bytes long, including the terminating null byte.
    int ret = prctl(PR_SET_NAME, name);
    if (ret != 0)
    {
        LOG_ERROR << "PR_SET_NAME failed, ret=" << ret;
    }
}

void panic(const char* title, const char* msg)
{
    fprintf(stderr, "PANIC! [%s] %s\n", title, msg); 
    abort(); 
}

Path GetExecutablePath()
{
    char fullpath[256] = { 0 };

    char process_path[] = "/proc/self/exe";
    const auto bytes = std::min(readlink(process_path, fullpath, sizeof(fullpath)), static_cast<ssize_t>(sizeof(fullpath) - 1));
    if (bytes >= 0)
        fullpath[bytes] = '\0';

    return fs::path(fullpath).parent_path();
}

long long getFileLastWriteTime(const Path& p)
{
    struct stat sb;
    int ret = stat(p.native().c_str(), &sb);
    if (ret != 0)
    {
        LOG_ERROR << "stat error, ret=" << ret;
        return 0;
    }
    return static_cast<long long>(sb.st_mtim.tv_sec);
}

namespace portable_strerror_r_detail {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

// Use the output of XSI-compliant (from POSIX.1-2001) strerror_r().
static char* impl(int strerror_r_ret, char* buffer, const size_t buffer_length)
{
    if (strerror_r_ret != 0) {
        // The message format follows what GNU's strerror_r() would
        // return in such case.
        snprintf(buffer, buffer_length, "Unknown error %d", strerror_r_ret);
    }
    return buffer;
}

// Use the output of GNU-specific strerror_r().
static char* impl(char* strerror_r_ret, char* /*buffer*/, const size_t /*buffer_length*/)
{
    return strerror_r_ret;
}

#pragma GCC diagnostic pop
} // namespace portable_strerror_r_detail

// Interface to support both GNU's and XSI's (from POSIX.1-2001) strerror_r().
const char* safe_strerror(const int errnum, char* buffer, const size_t buffer_length)
{
    return portable_strerror_r_detail::impl(strerror_r(errnum, buffer, buffer_length), buffer, buffer_length);
}

bool lunaticvibes::open(const std::string& link)
{
    const std::string s = (boost::format("xdg-open \"%s\"") % link).str();
    int rc = system(s.c_str());
    if (rc != 0)
    {
        LOG_ERROR << "xdg-open failed with rc=" << rc;
        return false;
    }
    return true;
}

#endif // __linux__
