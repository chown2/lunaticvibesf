#pragma once

#include <ostream>

#define PLOG_CAPTURE_FILE
#include <plog/Log.h> // IWYU pragma: export

int InitLogger();
int FreeLogger();

namespace lunaticvibes {

enum class LogLevel
{
    Fatal,
    Error,
    Warning,
    Info,
    Debug,
    Verbose,
};
std::ostream& operator<<(std::ostream& os, const LogLevel&);

void SetLogLevel(LogLevel level);

} // namespace lunaticvibes
