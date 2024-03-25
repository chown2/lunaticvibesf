#include "log.h"

#include <memory>
#include <ostream>
#include <sstream>

#include "common/utils.h"
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>

#include <plog/Appenders/ColorConsoleAppender.h>        // for command prompt log purpose
#include <plog/Appenders/RollingFileAppender.h>

namespace plog
{
    // TxtFormatterImpl
    class TxtFormatterFileLine
    {
    public:
        static util::nstring header()
        {
            return util::nstring();
        }

        static util::nstring format(const Record& record)
        {
            tm t;
            util::localtime_s(&t, &record.getTime().time);

            util::nostringstream ss;
            ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday << PLOG_NSTR(" ");
            ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3) << static_cast<int> (record.getTime().millitm) << PLOG_NSTR(" ");
            ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity()) << PLOG_NSTR(" ");
            ss << PLOG_NSTR("[") << record.getTid() << PLOG_NSTR("] ");
            ss << PLOG_NSTR("[") << fs::path(record.getFile()).filename() << PLOG_NSTR("@") << record.getLine() << PLOG_NSTR("] ");
            ss << record.getMessage() << PLOG_NSTR("\n");

            return ss.str();
        }
    };
}

std::shared_ptr<plog::ColorConsoleAppender<plog::TxtFormatterFileLine>> pConsoleAppender;
std::shared_ptr<plog::RollingFileAppender<plog::TxtFormatterFileLine>> pTxtAppender;
int InitLogger()
{
    pConsoleAppender = std::make_shared<plog::ColorConsoleAppender<plog::TxtFormatterFileLine>>();
    plog::init(plog::info, &*pConsoleAppender);

    /*
    char buf[128] = { 0 };
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    strftime(buf, sizeof(buf), "%F", std::gmtime(&t));
    std::stringstream logfile;
    logfile << PROJECT_NAME << "-" << buf << ".log";
    static auto txtAppender = plog::RollingFileAppender<plog::TxtFormatterImpl<false>>{ logfile.str().c_str(), 1000000, 5 };
    */
    pTxtAppender = std::make_shared<plog::RollingFileAppender<plog::TxtFormatterFileLine>>("LunaticVibesF.log", 1000000, 5);
    plog::get()->addAppender(&*pTxtAppender);

    return 0;
}

namespace lunaticvibes {

static plog::Severity plogSeverityFromLogLevel(const LogLevel level)
{
    switch (level)
    {
    case LogLevel::Fatal: return plog::fatal;
    case LogLevel::Error: return plog::error;
    case LogLevel::Warning: return plog::warning;
    case LogLevel::Info: return plog::info;
    case LogLevel::Debug: return plog::debug;
    case LogLevel::Verbose: return plog::verbose;
    }
    // Normally unreachable.
    return plog::verbose;
}

std::ostream& operator<<(std::ostream& os, const LogLevel& level)
{
    switch (level)
    {
    case LogLevel::Fatal: return os << "Fatal";
    case LogLevel::Error: return os << "Error";
    case LogLevel::Warning: return os << "Warning";
    case LogLevel::Info: return os << "Info";
    case LogLevel::Debug: return os << "Debug";
    case LogLevel::Verbose: return os << "Verbose";
    }
    // Normally unreachable.
    return os << "INVALID";
}

void SetLogLevel(const LogLevel level)
{
    plog::get()->setMaxSeverity(plogSeverityFromLogLevel(level));
    LOG_INFO << "[Log] log level set to " << level;
}

} // namespace lunaticvibes

int FreeLogger()
{
    pTxtAppender.reset();
    pConsoleAppender.reset();
    return 0;
}