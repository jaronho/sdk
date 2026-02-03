#include <atomic>
#include <stdio.h>
#include <string>
#include <thread>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include "../npacket/analyzer.h"
#include "../npacket/device/pcap_device.h"

static std::shared_ptr<npacket::Analyzer> s_inPktAnalyzer = nullptr; /* 接收包分析器 */
static npacket::PcapDevice s_inPacpDevice; /* 接收抓包设备 */
static std::atomic<uint64_t> s_inPkt{0}; /* 接收包总数 */
static std::atomic<uint64_t> s_inByte{0}; /* 接收字节总数 */
static std::atomic<uint64_t> s_inTcpPkt{0}; /* 接收TCP包总数 */
static std::atomic<uint64_t> s_inTcpByte{0}; /* 接收TCP字节总数 */
static std::atomic<uint64_t> s_inUdpPkt{0}; /* 接收UDP包总数 */
static std::atomic<uint64_t> s_inUdpByte{0}; /* 接收UDP字节总数 */

static std::shared_ptr<npacket::Analyzer> s_outPktAnalyzer = nullptr; /* 发送包分析器 */
static npacket::PcapDevice s_outPacpDevice; /* 发送抓包设备 */
static std::atomic<uint64_t> s_outPkt{0}; /* 发送包总数 */
static std::atomic<uint64_t> s_outByte{0}; /* 发送字节总数 */
static std::atomic<uint64_t> s_outTcpPkt{0}; /* 发送TCP包总数 */
static std::atomic<uint64_t> s_outTcpByte{0}; /* 发送TCP字节总数 */
static std::atomic<uint64_t> s_outUdpPkt{0}; /* 发送UDP包总数 */
static std::atomic<uint64_t> s_outUdpByte{0}; /* 发送UDP字节总数 */

std::string getDateTime()
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
    char buf1[20] = {0};
    strftime(buf1, sizeof(buf1), "%Y-%m-%d %H:%M:%S", &t);
    char buf2[4] = {0};
#ifdef _WIN32
    sprintf_s(buf2, sizeof(buf2), "%03d", milliseconds);
#else
    sprintf(buf2, "%03d", milliseconds);
#endif
    return std::string(buf1).append(".").append(buf2);
}

/**
 * @brief 处理接收以太网层
 */
bool handleInEthernetLayer(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                           const npacket::ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen)
{
    ++s_inPkt;
    s_inByte += totalLen;
    return true;
}

/**
 * @brief 处理接收传输层
 */
bool handleInTransportLayer(size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                            const npacket::ProtocolHeader* header, const uint8_t* payload, uint32_t payloadLen)
{
    switch ((npacket::TransportProtocol)header->getProtocol())
    {
    case npacket::TransportProtocol::TCP: {
        ++s_inTcpPkt;
        s_inTcpByte += totalLen;
    }
    break;
    case npacket::TransportProtocol::UDP: {
        ++s_inUdpPkt;
        s_inUdpByte += totalLen;
    }
    break;
    }
    return false;
}

/**
 * @brief 处理发送以太网层
 */
bool handleOutEthernetLayer(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const npacket::ProtocolHeader* header,
                            const uint8_t* payload, uint32_t payloadLen)
{
    ++s_outPkt;
    s_outByte += totalLen;
    return true;
}

/**
 * @brief 处理发送传输层
 */
bool handleOutTransportLayer(const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen, const npacket::ProtocolHeader* header,
                             const uint8_t* payload, uint32_t payloadLen)
{
    switch ((npacket::TransportProtocol)header->getProtocol())
    {
    case npacket::TransportProtocol::TCP: {
        ++s_outTcpPkt;
        s_outTcpByte += totalLen;
    }
    break;
    case npacket::TransportProtocol::UDP: {
        ++s_outUdpPkt;
        s_outUdpByte += totalLen;
    }
    break;
    }
    return false;
}

/**
 * @brief 响应发送线程
 */
void onInThread()
{
    while (1)
    {
        try
        {
            while (1)
            {
                s_inPacpDevice.captureOnce();
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
        catch (const std::exception& e)
        {
            printf("抓取接收数据异常: %s\n", e.what());
        }
        catch (...)
        {
            printf("抓取接收数据异常: 未知错误\n");
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

/**
 * @brief 响应接收线程
 */
void onOutThread()
{
    while (1)
    {
        try
        {
            while (1)
            {
                s_outPacpDevice.captureOnce();
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
        catch (const std::exception& e)
        {
            printf("抓取发送数据异常: %s\n", e.what());
        }
        catch (...)
        {
            printf("抓取发送数据异常: 未知错误\n");
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main(int argc, char* argv[])
{
    printf("*************************************************************************************************************\n");
    printf("** 说明: 网络状态统计工具.                                                                                 **\n");
    printf("**                                                                                                         **\n");
    printf("** 选项:                                                                                                   **\n");
    printf("**                                                                                                         **\n");
    printf("** [-l]                显示所有网络设备.                                                                   **\n");
#ifdef _WIN32
    printf("** [-i 名称]           网络设备名, 例如: {873ADB71-0F07-4857-9482-50B4DE0F6A68}等.                         **\n");
#else
    printf("** [-i 名称]           网络设备名, 例如: br0, enp1s0等.                                                    **\n");
#endif
    printf("**                                                                                                         **\n");
    printf("** 示例:                                                                                                   **\n");
    printf("**       netstat_tool.exe -i enp2s0                                                                        **\n");
    printf("**                                                                                                         **\n");
    printf("*************************************************************************************************************\n");
    printf("\n");
    bool showList = false;
    std::string name;
    for (int i = 1; i < argc;)
    {
        std::string key = argv[i];
        if (0 == key.compare("-l")) /* 显示设备列表 */
        {
            showList = true;
            i += 1;
        }
        else if (0 == key.compare("-i")) /* 设备接口 */
        {
            ++i;
            if (i < argc)
            {
                name = argv[i];
                ++i;
            }
        }
    }
    auto devList = npacket::PcapDevice::getAllDevices();
    if (showList)
    {
        printf("==================== 当前识别到的所有网卡设备 ====================\n");
        for (size_t i = 0, devCount = devList.size(); i < devCount; ++i)
        {
            if (i > 0)
            {
                printf("----------------------------------------\n");
            }
            printf("[%02zu] 设备名: %s\n", (i + 1), devList[i]->getName().c_str());
            printf("     描  述: %s\n", devList[i]->getDescribe().c_str());
            printf("       IPv4: %s\n", devList[i]->getIpv4Address().c_str());
            if (i == devCount - 1)
            {
                printf("----------------------------------------\n");
            }
        }
        printf("\n");
    }
    if (name.empty())
    {
        printf("未指定要监听的设备名\n");
        return 0;
    }
    printf("设备名: %s\n", name.c_str());
    /* 接收包 */
    npacket::CallbackConfig inCbCfg;
    inCbCfg.ethernetLayerCb = handleInEthernetLayer;
    inCbCfg.networkLayerCb = nullptr;
    inCbCfg.transportLayerCb = handleInTransportLayer;
    s_inPktAnalyzer = std::make_shared<npacket::Analyzer>(inCbCfg);
    if (!s_inPacpDevice.open(name, 1, 0, 0))
    {
#ifdef _WIN32
        printf("设备打开失败\n");
#else
        printf("接收包设备打开失败\n");
#endif
        return 0;
    }
    s_inPacpDevice.setDataCallback([&](const unsigned char* data, int dataLen) {
        static size_t num = 1;
        s_inPktAnalyzer->parse(num++, data, dataLen);
    });
    s_inPacpDevice.startCapture();
#ifndef _WIN32
    /* 发送包 */
    npacket::CallbackConfig outCbCfg;
    outCbCfg.ethernetLayerCb = handleOutEthernetLayer;
    outCbCfg.networkLayerCb = nullptr;
    outCbCfg.transportLayerCb = handleOutTransportLayer;
    s_outPktAnalyzer = std::make_shared<npacket::Analyzer>(outCbCfg);
    if (!s_outPacpDevice.open(name, 2, 0, 0))
    {
        printf("发送包设备打开失败\n");
        return 0;
    }
    s_outPacpDevice.setDataCallback([&](const unsigned char* data, int dataLen) {
        static size_t num = 1;
        s_outPktAnalyzer->parse(num++, data, dataLen);
    });
    s_outPacpDevice.startCapture();
#endif
    /* 创建抓包线程 */
    std::thread([&] { onInThread(); }).detach();
#ifndef _WIN32
    std::thread([&] { onOutThread(); }).detach();
#endif
    /* 主循环 */
    printf("\n");
    std::chrono::steady_clock::time_point lastSecondTimePoint{}; /* 上一秒时间点 */
    uint64_t lastInByte = 0, lastOutByte = 0; /* 上一秒接收/发送字节总数 */
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        /* 计算间隔1秒 */
        auto npt = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(npt - lastSecondTimePoint).count() >= 995)
        {
            lastSecondTimePoint = npt;
            auto deltaInByte = s_inByte - lastInByte; /* 过去一秒接收字节数 */
            auto deltaOutByte = s_outByte - lastOutByte; /* 过去一秒发送字节数 */
            lastInByte += deltaInByte;
            lastOutByte += deltaOutByte;
            /* 打印网络流量状态 */
            printf("[%s]\n", getDateTime().c_str());
#ifdef _WIN32
            printf("    速率: %zu B/s, 包总数: %zu, 字节总数: %zu, TCP包总数: %zu, TCP字节总数: %zu, UDP包总数: %zu, "
                   "UDP字节总数: %zu\n",
                   deltaInByte, s_inPkt.load(), s_inByte.load(), s_inTcpPkt.load(), s_inTcpByte.load(), s_inUdpPkt.load(),
                   s_inUdpByte.load());
#else
            printf("    接收 ↓ 速率: %zu B/s, 包总数: %zu, 字节总数: %zu, TCP包总数: %zu, TCP字节总数: %zu, UDP包总数: %zu, "
                   "UDP字节总数: %zu\n",
                   deltaInByte, s_inPkt.load(), s_inByte.load(), s_inTcpPkt.load(), s_inTcpByte.load(), s_inUdpPkt.load(),
                   s_inUdpByte.load());
            printf("    发送 ↑ 速率: %zu B/s, 包总数: %zu, 字节总数: %zu, TCP包总数: %zu, TCP字节总数: %zu, UDP包总数: %zu, "
                   "UDP字节总数: %zu\n",
                   deltaOutByte, s_outPkt.load(), s_outByte.load(), s_outTcpPkt.load(), s_outTcpByte.load(), s_outUdpPkt.load(),
                   s_outUdpByte.load());
#endif
        }
    }
    return 0;
}
