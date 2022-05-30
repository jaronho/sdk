#include "inner_logger_impl.h"

#include <fmt/color.h>
#include <sys/timeb.h>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace logger
{
struct DateTime
{
    char ymd[12]; /* 年月日 */
    char hms[12]; /* 时分秒 */
    char ms[4]; /* 毫秒 */
};

DateTime getDateTime()
{
    struct tm t;
    time_t now;
    time(&now);
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    t = *localtime(&now);
#endif
    DateTime dt;
    strftime(dt.ymd, sizeof(dt.ymd), "%Y-%m-%d", &t);
    strftime(dt.hms, sizeof(dt.hms), "%H:%M:%S", &t);
    struct timeb tb;
    ftime(&tb);
#ifdef _WIN32
    sprintf_s(dt.ms, sizeof(dt.ms), "%03d", tb.millitm);
#else
    sprintf(dt.ms, "%03d", tb.millitm);
#endif
    return dt;
}

std::string getLevelShortName(int level)
{
    switch (level)
    {
    case LEVEL_TRACE:
        return std::string("T");
    case LEVEL_DEBUG:
        return std::string("D");
    case LEVEL_INFO:
        return std::string("I");
    case LEVEL_WARN:
        return std::string("W");
    case LEVEL_ERROR:
        return std::string("E");
    case LEVEL_FATAL:
        return std::string("F");
    }
    return std::to_string(level);
}

fmt::v7::text_style getLevelTextStyle(const int level)
{
    switch (level)
    {
    case LEVEL_TRACE:
        return fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(255, 77, 222)));
    case LEVEL_DEBUG:
        return fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(75, 204, 239)));
    case LEVEL_INFO:
        return fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(236, 236, 236)));
    case LEVEL_WARN:
        return fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(222, 220, 21)));
    case LEVEL_ERROR:
        return fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(255, 128, 114)));
    case LEVEL_FATAL:
        return fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(255, 0, 0)));
    }
    return fmt::v7::fg(fmt::v7::color::white);
}

int getProcessId()
{
#ifdef _WIN32
    return _getpid();
#else
    return (int)getpid();
#endif
}

int getThreadId()
{
#ifdef _WIN32
    return GetCurrentThreadId();
#else
    return syscall(__NR_gettid);
#endif
}

InnerLoggerImpl::InnerLoggerImpl(const LogConfig& cfg) : InnerLogger(cfg.path, cfg.name)
{
    m_dailyLog = std::make_shared<DailyLogfile>(cfg.path, cfg.name, cfg.fileExtName, cfg.fileMaxSize, cfg.fileMaxCount, cfg.fileIndexFixed,
                                                cfg.newFolderDaily);
    m_level.store(cfg.level);
    m_consoleEnable.store(cfg.consoleEnable);
}

int InnerLoggerImpl::getLevel() const
{
    return m_level.load();
}

void InnerLoggerImpl::setLevel(int level)
{
    if (level != m_level.load())
    {
        m_level.store(level);
    }
}

bool InnerLoggerImpl::isConsoleEnable() const
{
    return m_consoleEnable.load();
}

void InnerLoggerImpl::setConsoleEnable(bool enable)
{
    m_consoleEnable.store(enable);
}

void InnerLoggerImpl::print(int level, const std::string& tag, const std::string& file, int line, const std::string& func,
                            const std::string& msg)
{
    static const std::string PID = std::to_string(getProcessId());
    DateTime dt = getDateTime();
    /* 拼接日志内容 */
    std::string content;
    content.append(getLevelShortName(level)); /* 级别 */
    content.append("[").append(dt.ymd).append(" ").append(dt.hms).append(".").append(dt.ms).append("]"); /* 时间 */
    content.append("[").append(PID).append(":").append(std::to_string(getThreadId())).append("]"); /* 进程:线程 */
    if (!tag.empty())
    {
        content.append("[").append(tag).append("]"); /* 标签 */
    }
    if (!file.empty()) /* 文件名 行号 */
    {
        content.append("[").append(file).append(" ").append(std::to_string(line));
        if (!func.empty()) /* 函数名 */
        {
            content.append(" ").append(func);
        }
        content.append("]");
    }
    else if (!func.empty()) /* 函数名 */
    {
        content.append("[").append(func).append("]");
    }
    content.append(" ").append(msg); /* 内容 */
    /* 日志记录/打印 */
    if (level >= m_level)
    {
        /* 记录到文件 */
        m_dailyLog->record(content, true);
        /* 打印到控制台 */
        if (m_consoleEnable)
        {
            fmt::print(getLevelTextStyle(level), "{}\n", content);
        }
    }
}
} // namespace logger
