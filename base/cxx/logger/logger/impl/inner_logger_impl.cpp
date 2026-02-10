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
inline int getProcessId()
{
#ifdef _WIN32
    return _getpid();
#else
    return (int)getpid();
#endif
}

inline int getThreadId()
{
#ifdef _WIN32
    return GetCurrentThreadId();
#else
    return syscall(__NR_gettid);
#endif
}

struct DateTime
{
    char ymd[12]; /* 年月日 */
    char hms[12]; /* 时分秒 */
    char ms[4]; /* 毫秒 */
};

inline DateTime& getDateTime()
{
    static thread_local DateTime dt;
    static thread_local uint64_t lastMs = 0;
    struct timeval tv;
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    tv.tv_sec = time(nullptr);
    tv.tv_usec = st.wMilliseconds * 1000;
#else
    gettimeofday(&tv, nullptr);
#endif
    uint64_t nowMs = (uint64_t)(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
    if (nowMs != lastMs)
    {
        lastMs = nowMs;
        struct tm t;
#ifdef _WIN32
        localtime_s(&t, (const time_t*)(&tv.tv_sec));
#else
        localtime_r((const time_t*)(&tv.tv_sec), &t);
#endif
        /* 手动格式化, 避免strftime */
        auto itoa2 = [](int v, char* p) {
            *p = (char)('0' + v / 10);
            *(p + 1) = (char)('0' + v % 10);
        };
        auto itoa4 = [](int v, char* p) {
            *(p + 3) = (char)('0' + v % 10);
            v /= 10;
            *(p + 2) = (char)('0' + v % 10);
            v /= 10;
            *(p + 1) = (char)('0' + v % 10);
            v /= 10;
            *p = (char)('0' + v);
        };
        itoa4(t.tm_year + 1900, dt.ymd);
        dt.ymd[4] = '-';
        itoa2(t.tm_mon + 1, dt.ymd + 5);
        dt.ymd[7] = '-';
        itoa2(t.tm_mday, dt.ymd + 8);
        dt.ymd[10] = '\0';
        itoa2(t.tm_hour, dt.hms);
        dt.hms[2] = ':';
        itoa2(t.tm_min, dt.hms + 3);
        dt.hms[5] = ':';
        itoa2(t.tm_sec, dt.hms + 6);
        dt.hms[8] = '\0';
        int ms = tv.tv_usec / 1000;
        dt.ms[0] = (char)('0' + ms / 100);
        dt.ms[1] = (char)('0' + (ms / 10) % 10);
        dt.ms[2] = (char)('0' + ms % 10);
        dt.ms[3] = '\0';
    }
    return dt;
}

inline char getLevelShortName(int level)
{
    static const char LEVEL_NAME[] = "TDIWEF";
    return (level >= 0 && level <= 5) ? LEVEL_NAME[level] : (char)('0' + level);
}

inline fmt::v7::text_style getLevelTextStyle(const int level)
{
    static const fmt::v7::text_style LEVEL_STYLES[] = {
        fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(255, 77, 222))),
        fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(75, 204, 239))),
        fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(236, 236, 236))),
        fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(222, 220, 21))),
        fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(255, 128, 114))),
        fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(255, 0, 0))),
    };
    return (level >= 0 && level <= 5) ? LEVEL_STYLES[level] : fmt::v7::fg(fmt::v7::color::white);
}

inline void appendChar(char*& p, char c)
{
    *p++ = c;
}

inline void appendCstr(char*& p, const char* s, size_t n)
{
    memcpy(p, s, n);
    p += n;
}

inline void appendString(char*& p, const std::string& s)
{
    memcpy(p, s.data(), s.size());
    p += s.size();
}

inline void appendInt(char*& p, int v)
{
    if (0 == v)
    {
        *p++ = '0';
        return;
    }
    if (v < 0)
    {
        *p++ = '-';
        v = -v;
    }
    if (v < 1000) /* 小整数展开 */
    {
        if (v >= 100)
        {
            *p++ = (char)('0' + v / 100);
            v %= 100;
            *p++ = (char)('0' + v / 10);
            *p++ = (char)('0' + v % 10);
        }
        else if (v >= 10)
        {
            *p++ = (char)('0' + v / 10);
            *p++ = (char)('0' + v % 10);
        }
        else
        {
            *p++ = (char)('0' + v);
        }
        return;
    }
    /* 大整数通用实现 */
    char tmp[16];
    int i = 0;
    while (v > 0)
    {
        tmp[i++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while (i--)
    {
        *p++ = tmp[i];
    }
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
    if (level < m_level.load(std::memory_order_relaxed))
    {
        return;
    }
    /* 静态缓存 */
    static const int PID = getProcessId();
    static thread_local int TID = getThreadId();
    static const size_t STACK_BUF_SIZE = 4096;
    static thread_local char stackBuf[STACK_BUF_SIZE];
    static thread_local std::string heapBuf;
    static thread_local size_t heapCapacity = 0;
    char* buf = stackBuf;
    bool useHeap = false;
    size_t totalLen = 512 + msg.size(); /* 预计算总长度 */
    if (totalLen > STACK_BUF_SIZE)
    {
        if (heapCapacity < totalLen)
        {
            heapBuf.reserve(totalLen);
            heapCapacity = heapBuf.capacity();
        }
        heapBuf.clear();
        buf = (char*)heapBuf.data();
        useHeap = true;
    }
    char* p = buf;
    DateTime& dt = getDateTime();
    /* 级别 */
    appendChar(p, getLevelShortName(level));
    /* 时间, [YYYY-MM-DD HH:MM:SS.mmm] */
    appendChar(p, '[');
    appendCstr(p, dt.ymd, 10);
    appendChar(p, ' ');
    appendCstr(p, dt.hms, 8);
    appendChar(p, '.');
    appendCstr(p, dt.ms, 3);
    appendChar(p, ']');
    /* [进程:线程] */
    appendChar(p, '[');
    appendInt(p, PID);
    appendChar(p, ':');
    appendInt(p, TID);
    appendChar(p, ']');
    /* [标签] */
    if (!tag.empty())
    {
        appendChar(p, '[');
        appendString(p, tag);
        appendChar(p, ']');
    }
    /* [文件名 函数名 行号] */
    if (!file.empty() || !func.empty())
    {
        appendChar(p, '[');
        if (!file.empty())
        {
            appendString(p, file);
            if (!func.empty())
            {
                appendChar(p, ' ');
            }
        }
        if (!func.empty())
        {
            appendString(p, func);
        }
        appendChar(p, ' ');
        appendInt(p, line);
        appendChar(p, ']');
    }
    /* 内容 */
    appendChar(p, ' ');
    appendString(p, msg);
    size_t bufLen = (size_t)(p - buf);
    /* 记录到文件 */
    auto dailyLog = getDailyLog(level);
    if (dailyLog)
    {
        bool immediateFlush = (level >= m_flushLevel);
        if (useHeap)
        {
            dailyLog->record(std::move(heapBuf), immediateFlush); /* 直接移动(零拷贝) */
            heapCapacity = 0; /* 标记已转移, 下次重新分配 */
        }
        else
        {
            dailyLog->record(buf, bufLen, immediateFlush);
        }
    }
    /* 打印到控制台 */
    if (1 == m_consoleMode)
    {
        fwrite(buf, 1, bufLen, stdout);
        fwrite("\n", 1, 1, stdout);
    }
    else if (2 == m_consoleMode)
    {
        fmt::print(getLevelTextStyle(level), "{}\n", fmt::v7::string_view(buf, bufLen));
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
