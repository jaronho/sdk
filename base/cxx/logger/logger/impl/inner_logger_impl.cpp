#include "inner_logger_impl.h"

#include <fmt/color.h>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#else
#include <sys/syscall.h>
#include <sys/time.h>
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
#ifdef _WIN32
    SYSTEMTIME now;
    GetLocalTime(&now);
    t.tm_year = now.wYear - 1900;
    t.tm_mon = now.wMonth - 1;
    t.tm_mday = now.wDay;
    t.tm_hour = now.wHour;
    t.tm_min = now.wMinute;
    t.tm_sec = now.wSecond;
    long milliseconds = now.wMilliseconds;
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    localtime_r(&now.tv_sec, &t);
    long milliseconds = now.tv_usec / 1000;
#endif
    DateTime dt;
    strftime(dt.ymd, sizeof(dt.ymd), "%Y-%m-%d", &t);
    strftime(dt.hms, sizeof(dt.hms), "%H:%M:%S", &t);
#ifdef _WIN32
    sprintf_s(dt.ms, sizeof(dt.ms), "%03d", milliseconds);
#else
    sprintf(dt.ms, "%03d", milliseconds);
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
    if (!cfg.path.empty())
    {
        m_dailyLog = std::make_shared<DailyLogfile>(cfg.path, cfg.name, "", cfg.fileExtName, cfg.fileMaxSize, cfg.fileMaxCount,
                                                    cfg.fileIndexFixed, cfg.newFolderDaily);
        m_dailyLogTrace = std::make_shared<DailyLogfile>(cfg.path, cfg.name, "_trace", cfg.fileExtName, cfg.fileMaxSize, cfg.fileMaxCount,
                                                         cfg.fileIndexFixed, cfg.newFolderDaily);
        m_dailyLogDebug = std::make_shared<DailyLogfile>(cfg.path, cfg.name, "_debug", cfg.fileExtName, cfg.fileMaxSize, cfg.fileMaxCount,
                                                         cfg.fileIndexFixed, cfg.newFolderDaily);
        m_dailyLogInfo = std::make_shared<DailyLogfile>(cfg.path, cfg.name, "_info", cfg.fileExtName, cfg.fileMaxSize, cfg.fileMaxCount,
                                                        cfg.fileIndexFixed, cfg.newFolderDaily);
        m_dailyLogWarn = std::make_shared<DailyLogfile>(cfg.path, cfg.name, "_warn", cfg.fileExtName, cfg.fileMaxSize, cfg.fileMaxCount,
                                                        cfg.fileIndexFixed, cfg.newFolderDaily);
        m_dailyLogError = std::make_shared<DailyLogfile>(cfg.path, cfg.name, "_error", cfg.fileExtName, cfg.fileMaxSize, cfg.fileMaxCount,
                                                         cfg.fileIndexFixed, cfg.newFolderDaily);
        m_dailyLogFatal = std::make_shared<DailyLogfile>(cfg.path, cfg.name, "_fatal", cfg.fileExtName, cfg.fileMaxSize, cfg.fileMaxCount,
                                                         cfg.fileIndexFixed, cfg.newFolderDaily);
    }
    m_fileMaxSize = cfg.fileMaxSize;
    m_fileMaxCount = cfg.fileMaxCount;
    m_level = cfg.level;
    {
        std::lock_guard<std::mutex> locker(m_mutexLevelFile);
        m_levelFile = cfg.levelFile;
    }
    m_flushLevel = cfg.flushLevel;
    m_consoleMode = cfg.consoleMode;
}

size_t InnerLoggerImpl::getMaxSize()
{
    return m_fileMaxSize;
}

void InnerLoggerImpl::setMaxSize(size_t maxSize)
{
    m_fileMaxSize = maxSize;
    if (m_dailyLog)
    {
        m_dailyLog->setMaxSize(maxSize);
    }
    if (m_dailyLogTrace)
    {
        m_dailyLogTrace->setMaxSize(maxSize);
    }
    if (m_dailyLogDebug)
    {
        m_dailyLogDebug->setMaxSize(maxSize);
    }
    if (m_dailyLogInfo)
    {
        m_dailyLogInfo->setMaxSize(maxSize);
    }
    if (m_dailyLogWarn)
    {
        m_dailyLogWarn->setMaxSize(maxSize);
    }
    if (m_dailyLogError)
    {
        m_dailyLogError->setMaxSize(maxSize);
    }
    if (m_dailyLogFatal)
    {
        m_dailyLogFatal->setMaxSize(maxSize);
    }
}

size_t InnerLoggerImpl::getMaxFiles()
{
    return m_fileMaxCount;
}

void InnerLoggerImpl::setMaxFiles(size_t maxFiles)
{
    m_fileMaxCount = maxFiles;
    if (m_dailyLog)
    {
        m_dailyLog->setMaxFiles(maxFiles);
    }
    if (m_dailyLogTrace)
    {
        m_dailyLogTrace->setMaxFiles(maxFiles);
    }
    if (m_dailyLogDebug)
    {
        m_dailyLogDebug->setMaxFiles(maxFiles);
    }
    if (m_dailyLogInfo)
    {
        m_dailyLogInfo->setMaxFiles(maxFiles);
    }
    if (m_dailyLogWarn)
    {
        m_dailyLogWarn->setMaxFiles(maxFiles);
    }
    if (m_dailyLogError)
    {
        m_dailyLogError->setMaxFiles(maxFiles);
    }
    if (m_dailyLogFatal)
    {
        m_dailyLogFatal->setMaxFiles(maxFiles);
    }
}

int InnerLoggerImpl::getLevel()
{
    return m_level;
}

void InnerLoggerImpl::setLevel(int level)
{
    m_level = level;
}

int InnerLoggerImpl::getLevelFile(int level)
{
    std::lock_guard<std::mutex> locker(m_mutexLevelFile);
    auto iter = m_levelFile.find(level);
    return ((m_levelFile.end() == iter) ? -1 : iter->second);
}

void InnerLoggerImpl::setLevelFile(int level, int fileType)
{
    std::lock_guard<std::mutex> locker(m_mutexLevelFile);
    auto iter = m_levelFile.find(level);
    if (m_levelFile.end() == iter)
    {
        m_levelFile.insert(std::make_pair(level, fileType));
    }
    else
    {
        iter->second = fileType;
    }
}

int InnerLoggerImpl::getFlushLevel()
{
    return m_flushLevel;
}

void InnerLoggerImpl::setFlushLevel(int level)
{
    m_flushLevel = level;
}

int InnerLoggerImpl::getConsoleMode()
{
    return m_consoleMode;
}

void InnerLoggerImpl::setConsoleMode(int mode)
{
    if (mode < 0 || mode > 2)
    {
        mode = 0;
    }
    m_consoleMode = mode;
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
        auto dailyLog = getDailyLog(level);
        if (dailyLog)
        {
            bool immediateFlush = (level >= m_flushLevel);
            dailyLog->record(content, true, immediateFlush);
        }
        /* 打印到控制台 */
        if (1 == m_consoleMode)
        {
            fmt::print(fmt::v7::text_style(), "{}\n", content);
        }
        else if (2 == m_consoleMode)
        {
            fmt::print(getLevelTextStyle(level), "{}\n", content);
        }
    }
}

void InnerLoggerImpl::forceFlush()
{
    if (m_dailyLog)
    {
        m_dailyLog->forceFlush();
    }
    if (m_dailyLogTrace)
    {
        m_dailyLogTrace->forceFlush();
    }
    if (m_dailyLogDebug)
    {
        m_dailyLogDebug->forceFlush();
    }
    if (m_dailyLogInfo)
    {
        m_dailyLogInfo->forceFlush();
    }
    if (m_dailyLogWarn)
    {
        m_dailyLogWarn->forceFlush();
    }
    if (m_dailyLogError)
    {
        m_dailyLogError->forceFlush();
    }
    if (m_dailyLogFatal)
    {
        m_dailyLogFatal->forceFlush();
    }
}

std::shared_ptr<DailyLogfile> InnerLoggerImpl::getDailyLog(int level)
{
    int fileType = -1;
    {
        std::lock_guard<std::mutex> locker(m_mutexLevelFile);
        auto iter = m_levelFile.find(level);
        if (m_levelFile.end() != iter)
        {
            fileType = iter->second;
        }
    }
    switch (fileType)
    {
    case LEVEL_TRACE:
        return m_dailyLogTrace;
    case LEVEL_DEBUG:
        return m_dailyLogDebug;
    case LEVEL_INFO:
        return m_dailyLogInfo;
    case LEVEL_WARN:
        return m_dailyLogWarn;
    case LEVEL_ERROR:
        return m_dailyLogError;
    case LEVEL_FATAL:
        return m_dailyLogFatal;
    }
    return m_dailyLog;
}
} // namespace logger
