#include <iostream>
#include <thread>

#include "timeconsuming/timeout_monitor.h"

/* ��ʱ�������ʼ */
void onDetectorBegin(const std::string& tag, const std::string& file, int line, const std::string& func)
{
    std::cout << "detector[start]: [" << tag << "] [" << file << " " << line << " " << func << "]" << std::endl;
}

/* ��ʱ�����ץ�� */
void onDetectorCapture(const std::string& tag, const std::string& subTag, const std::string& file, int line, const std::string& func,
                       const std::chrono::milliseconds& duration)
{
    std::cout << "detector[capture]: [" << tag << "][" << subTag << "] [" << file << " " << line << " " << func << " " << duration.count()
              << " ms]" << std::endl;
}

/* ��ʱ��������� */
void onDetectorEnd(const std::string& tag, const std::string& file, int line, const std::string& func,
                   const std::chrono::milliseconds& duration)
{
    std::cout << "detector[finish]: [" << tag << "] [" << file << " " << line << " " << func << "] [" << duration.count() << " ms]"
              << std::endl;
}

/* ��ʱ�����ץ�� */
void onMonitorCapture(const std::string& tag, const std::string& subTag, const std::string& file, int line, const std::string& func,
                      const std::chrono::milliseconds& duration)
{
    std::cout << "monitor[capture]: [" << tag << "][" << subTag << "] [" << file << " " << line << " " << func << "] [" << duration.count()
              << " ms]" << std::endl;
}

/* ��ʱ��������� */
void onMonitorEnd(const std::string& tag, const std::string& file, int line, const std::string& func,
                  const std::chrono::milliseconds& duration)
{
    std::cout << "monitor[finish]: [" << tag << "] [" << file << " " << line << " " << func << "] [" << duration.count() << " ms]"
              << std::endl;
}

/* ���Ժ�ʱ�����: ��ͨ�� */
void testTimeConsumeDetector1()
{
    TIME_CONSUMING_DETECTOR(detector, onDetectorBegin, onDetectorCapture, onDetectorEnd, "testTimeConsumeDetector1");
    for (int i = 0; i <= 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(i * 100));
        TIME_CONSUMING_DETECTOR_CAPTURE(detector, std::to_string(i));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/* ���Ժ�ʱ�����: �򻯺�ĺ� */
void testTimeConsumeDetector2()
{
    SIMPLE_TIME_CONSUMING_DETECTOR(onDetectorEnd, "testTimeConsumeDetector2");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/* ���Գ�ʱ�����: ��ͨ�� */
void testTimeoutMonitor1()
{
    START_TIMEOUT_MONITOR(monitor, onMonitorCapture, onMonitorEnd, std::chrono::seconds(1), "testTimeoutMonitor1");
    for (int i = 0; i <= 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(i * 100));
        TIMEOUT_MONITOR_CAPTURE(monitor, std::chrono::milliseconds(50), std::to_string(i));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/* ���Գ�ʱ�����: �򻯺�ĺ�, ���׵ĺ� */
void testTimeoutMonitor2()
{
    START_SIMPLE_TIMEOUT_MONITOR(onMonitorEnd, std::chrono::milliseconds(100), "testTimeoutMonitor2-1");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    START_EASY_TIMEOUT_MONITOR(onMonitorEnd, "testTimeoutMonitor2-2");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main()
{
    testTimeConsumeDetector1();
    testTimeConsumeDetector2();
    std::cout << std::endl << std::endl;
    testTimeoutMonitor1();
    testTimeoutMonitor2();
    return 0;
}
