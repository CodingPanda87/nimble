// *************************************************************************
// [Author]		xiong.qiang
// [Date]		2024-09-05
// [Describe]	Logging Interface
// [Copyright]  xiong.qiang
// [Brief]      Logging system interface definitions
// *************************************************************************
#pragma once

#include "x/x.hpp"
#include "itf.hpp"

namespace nb {

inline constexpr Version VER_LOG     = {1, 0, 0};
inline constexpr Version VER_LOG_MIN = {1, 0, 0};

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

inline x::str LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

inline LogLevel ParseLogLevel(const x::str& levelStr) {
    if (levelStr == "debug") return LogLevel::DEBUG;
    if (levelStr == "info")  return LogLevel::INFO;
    if (levelStr == "warn")  return LogLevel::WARN;
    if (levelStr == "error") return LogLevel::ERROR;
    if (levelStr == "fatal") return LogLevel::FATAL;
    return LogLevel::DEBUG;
}


struct LogItem{
    x::str      title;
    x::str      msg;
    x::str      codeInfo;
    x::Time     time;
    LogLevel    level = LogLevel::DEBUG;

    // [2023-07-18 12:34:56.000123][INFO][CodeInfo][Title]: This is a log message.
    x::str toString() const noexcept {
        return _fmt("[{}][{}][{}][{}]:{}", 
            time.to_string(),LogLevelToString(level),codeInfo,title,msg);
    }
};

class I_LogListener : public ITF {
public:
    virtual ~I_LogListener() = default;

    virtual void onLog(const LogItem& log) noexcept = 0;

protected:
    I_LogListener() : ITF("I_LogListener", VER_LOG, VER_LOG_MIN) {}
};

class I_Log : public ITF{
public:
    virtual ~I_Log() = default;

    virtual void        enable(bool en)                                          noexcept = 0;
    virtual void        setCache(int size)                                       noexcept = 0;     
    virtual x::Result   addFilter(x::cStr& filter)                               noexcept = 0;
    virtual void        addListener(x::cStr& key,I_LogListener* listener)        noexcept = 0;
    virtual void        flush()                                                  noexcept = 0;

    virtual void        debug(x::cStr& title,x::cStr& message,x::cStr& codeInfo) noexcept = 0;
    virtual void         info(x::cStr& title,x::cStr& message,x::cStr& codeInfo) noexcept = 0;
    virtual void         warn(x::cStr& title,x::cStr& message,x::cStr& codeInfo) noexcept = 0;
    virtual void        error(x::cStr& title,x::cStr& message,x::cStr& codeInfo) noexcept = 0;
    virtual void        fatal(x::cStr& title,x::cStr& message,x::cStr& codeInfo) noexcept = 0;

protected:
    I_Log() : ITF("I_Log", VER_LOG, VER_LOG_MIN) {}
};

extern I_Log* g_log;

#define LOG_DEBUG(title, message)                              nb::g_log->debug(title, message, _code_info())
#define LOG_INFO( title, message)                              nb::g_log->info( title, message, _code_info())
#define LOG_WARN( title, message)                              nb::g_log->warn( title, message, _code_info())
#define LOG_ERROR(title, message)                              nb::g_log->error(title, message, _code_info())
#define LOG_FATAL(title, message)                              nb::g_log->fatal(title, message, _code_info())

#define LOG_DEBUG_IF(condition, title, message) if (condition) nb::g_log->debug(title, message, _code_info())
#define LOG_INFO_IF( condition, title, message) if (condition) nb::g_log->info( title, message, _code_info())
#define LOG_WARN_IF( condition, title, message) if (condition) nb::g_log->warn( title, message, _code_info())
#define LOG_ERROR_IF(condition, title, message) if (condition) nb::g_log->error(title, message, _code_info())
#define LOG_FATAL_IF(condition, title, message) if (condition) nb::g_log->fatal(title, message, _code_info())

} // namespace nb
