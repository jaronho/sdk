#include "inner_logger_impl.h"

#include <fmt/color.h>
#include <sys/timeb.h>
#include <thread>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace logger
{
const std::string DAILY_LOG_EXT = ".log"; /* ��־�ļ�Ĭ�Ϻ�׺�� */
const size_t DAILY_LOG_MAX_SIZE = 20 * 1024 * 1024; /* ÿ����־�ļ��������(����Ĭ��Ϊ20M) */
const size_t DAILY_LOG_MAX_COUNT = 0U; /* ÿ������������ɵ���־�ļ���(����Ĭ�ϲ�����) */

struct DateTime
{
    char ymd[12]; /* ������ */
    char hms[12]; /* ʱ���� */
    char ms[4]; /* ���� */
};

DateTime getDateTime()
{
    struct tm t;
    time_t now;
    time(&now);
    localtime_s(&t, &now);
    DateTime dt;
    strftime(dt.ymd, sizeof(dt.ymd), "%Y-%m-%d", &t);
    strftime(dt.hms, sizeof(dt.hms), "%H:%M:%S", &t);
    struct timeb tb;
    ftime(&tb);
    sprintf_s(dt.ms, "%03d", tb.millitm);
    return dt;
}

std::string getLevelShortName(const Level& level)
{
    switch (level)
    {
    case Level::TRACE:
        return std::string("T");
    case Level::DEBUG:
        return std::string("D");
    case Level::INFO:
        return std::string("I");
    case Level::WARN:
        return std::string("W");
    case Level::ERROR:
        return std::string("E");
    case Level::FATAL:
        return std::string("F");
    }
    return std::to_string((int)level);
}

fmt::v7::text_style getLevelTextStyle(const Level& level)
{
    switch (level)
    {
    case Level::TRACE:
        return fmt::v7::fg(fmt::v7::color::white_smoke);
    case Level::DEBUG:
        return fmt::v7::fg(fmt::v7::color::white);
    case Level::INFO:
        return fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(127, 255, 212)));
    case Level::WARN:
        return fmt::v7::fg(fmt::v7::color::yellow);
    case Level::ERROR:
        return fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(238, 48, 167)));
    case Level::FATAL:
        return fmt::v7::fg(fmt::v7::detail::color_type(fmt::v7::rgb(255, 0, 0)));
    }
    return fmt::v7::fg(fmt::v7::color::white);
}

int getProcessId()
{
#if _WIN32
    return _getpid();
#else
    return (int)getpid();
#endif
}

int getThreadId()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return atoi(ss.str().c_str());
}

InnerLoggerImpl::InnerLoggerImpl(const std::string& path, const std::string& name) : InnerLogger(path, name)
{
    m_dailyLog = std::make_shared<DailyLogfile>(path, name, DAILY_LOG_EXT, DAILY_LOG_MAX_SIZE, DAILY_LOG_MAX_COUNT);
}

Level InnerLoggerImpl::getLevel() const
{
    return m_level.load();
}

void InnerLoggerImpl::setLevel(const Level& level)
{
    if (level != m_level)
    {
        m_level = level;
    }
}

bool InnerLoggerImpl::isConsoleEnable()
{
    return m_consoleEnable;
}

void InnerLoggerImpl::setConsoleEnable(bool enable)
{
    m_consoleEnable = true;
}

void InnerLoggerImpl::print(const Level& level, const std::string& tag, const std::string& file, int line, const std::string& func, const std::string& msg)
{
    static const std::string PID = std::to_string(getProcessId());
    DateTime dt = getDateTime();
    /* ƴ����־���� */
    std::string content;
    content.append(getLevelShortName(level)); /* ���� */
    content.append("[").append(dt.ymd).append(" ").append(dt.hms).append(".").append(dt.ms).append("]"); /* ʱ�� */
    content.append("[").append(PID).append(":").append(std::to_string(getThreadId())).append("]"); /* ����:�߳� */
    content.append("[").append(tag).append("]"); /* ��ǩ */
    if (!file.empty()) /* �ļ��� �к� ������ */
    {
        content.append("[").append(file).append(" ").append(std::to_string(line)).append(" ").append(func).append("]");
    }
    content.append(" ").append(msg); /* ���� */
    /* ��¼����־�ļ� */
    if (level >= m_level)
    {
        m_dailyLog->record(content, true);
    }
    /* ��ӡ������̨ */
    if (m_consoleEnable)
    {
        fmt::print(getLevelTextStyle(level), "{}\n", content);
    }
}
} // namespace logger
