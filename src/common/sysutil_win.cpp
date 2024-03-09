#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include "sysutil.h"

#include <cstdio>
#include <filesystem>

#include <VersionHelpers.h>
#include <shellapi.h>
#include <windows.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)  
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.  
    LPCSTR szName; // Pointer to name (in user addr space).  
    DWORD dwThreadID; // Thread ID (-1=caller thread).  
    DWORD dwFlags; // Reserved for future use, must be zero.  
} THREADNAME_INFO;
#pragma pack(pop)  

void SetThreadNameWin32(DWORD dwThreadID, const char* threadName) {

    // change VS debugger thread name
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
#pragma warning(push)  
#pragma warning(disable: 6320 6322)  
    __try {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)& info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
#pragma warning(pop)  
}

[[noreturn]] inline void panicWin32(const char* title, const char* msg)
{
    MessageBox(NULL, msg, title, MB_OK);
    abort();
}

static DWORD dwMainThreadId = 0;

void SetThreadAsMainThread()
{
    dwMainThreadId = GetCurrentThreadId();
}

int64_t GetCurrentThreadID()
{
    return GetCurrentThreadId();
}

bool IsMainThread()
{
    return GetCurrentThreadId() == dwMainThreadId;
}

void SetThreadName(const char* name)
{
    SetThreadNameWin32(GetCurrentThreadId(), name); 
}
void panic(const char* title, const char* msg) 
{
    panicWin32(title, msg);
}

Path GetExecutablePath()
{
    char fullpath[256] = { 0 };

    if (!GetModuleFileNameA(NULL, fullpath, sizeof(fullpath)))
    {
        return {};
    }

    return fs::path(fullpath).parent_path();
}

static HWND hwnd = NULL;
void setWindowHandle(void* handle)
{
    hwnd = *(HWND*)handle;
}
void getWindowHandle(void* handle)
{
    *(HWND*)handle = hwnd;
}

long long getFileLastWriteTime(const Path& p)
{
    return std::chrono::duration_cast<std::chrono::seconds>(fs::last_write_time(p).time_since_epoch()).count() - 11644473600;
}

const char* safe_strerror(int errnum, char* buffer, size_t buffer_length)
{
    strerror_s(buffer, buffer_length, errnum);
    return buffer;
}

bool lunaticvibes::open(const std::string& link)
{
    auto res = reinterpret_cast<INT_PTR>(ShellExecute(NULL, "open", link.c_str(), NULL, NULL, SW_SHOWDEFAULT));
    // > .. can be cast only to an INT_PTR
    // > If the function succeeds, it returns a value greater than 32.
    return res > 32;
}

#endif