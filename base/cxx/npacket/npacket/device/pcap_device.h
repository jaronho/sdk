#pragma once
#define _XKEYCHECK_H /* 防止在Windows平台编译报错 */
#include <functional>
#include <memory>
#include <mutex>
#include <pcap.h>
#include <string>
#include <vector>

namespace npacket
{
/**
 * @brief pcap设备(对网络适配器的抽象)
 */
class PcapDevice final
{
public:
    ~PcapDevice();

    /**
     * @brief 获取名字
     * @return 名字
     */
    std::string getName();

    /**
     * @brief 获取描述
     * @return 描述
     */
    std::string getDescribe();

    /**
     * @brief 获取IPv4地址
     * @return IPv4地址
     */
    std::string getIpv4Address();

    /**
     * @brief 是否回环
     * @return true-是, false-否
     */
    bool isLoopback();

    /**
     * @brief 打开
     * @param name 设备名, 例如: enp1s0
     * @param direction 要捕获的数据流向, 0-所有, 1-接收, 2-发送, 注意: 不支持Windows平台
     * @param snapLen 快照长度, 要捕获的数据包长度, 正常设置为65536能够满足所有网络
     * @param promisc 混杂模式, 该模式下适配器将接受所有数据包，即便那不是发到该适配器的, 0-普通模式, 1-混杂模式
     * @param timeout 超时读取, <=0表示永不超时
     * @param bufferSize 缓冲大小
     * @return true-成功, false-失败
     */
    bool open(const std::string& name, int direction = 0, int snapLen = 0, int promisc = 1, int timeout = 0, int bufferSize = 0);

    /**
     * @brief 设置BPF过滤
     * @param bpf 过滤表达式, 例如: "dst port 80"
     * @param optimize 是否需要优化过滤表达式, 值: 0-不优化, 1-优化
     * @param netmask 指定本地网络的网络掩码
     * @return true-成功, false-失败
     */
    bool setFilter(const std::string& bpf, int optimize = 1, int netmask = 0);

    /**
     * @brief 设置数据回调
     * @param cb 数据回调, 参数: data-数据, dataLen-数据长度
     */
    void setDataCallback(const std::function<void(const unsigned char* data, unsigned int dataLen)>& cb);

    /**
     * @brief 捕获单次(需要在循环调用)
     */
    bool captureOnce();

    /**
     * @brief 开始捕获
     * @return true-成功, false-失败
     */
    bool startCapture();

    /**
     * @brief 停止捕获
     */
    void stopCapture();

    /**
     * @brief 关闭
     */
    void close();

    /**
     * @brief 获取本机所有pcap设备
     * @return pcap设备列表
     */
    static std::vector<std::shared_ptr<PcapDevice>> getAllDevices(std::string* errorBuffer = nullptr);

private:
    /**
     * @brief 响应数据捕获
     * @param user 自定义用户数据
     * @param pkthdr 数据头
     * @param packet 数据包
     */
    static void onPacketArrived(uint8_t* user, const struct pcap_pkthdr* pkthdr, const uint8_t* packet);

private:
    std::string m_name; /* 名字 */
    std::string m_describe; /* 描述 */
    std::string m_ipv4Address; /* IPv4地址 */
    bool m_isLoopback = false; /* 是否回环 */
    std::recursive_mutex m_mutex;
    pcap_t* m_pcap = nullptr; /* pcap指针 */
    bool m_captureStarted = false; /* 是否已经开始捕获 */
    std::mutex m_mutexOnDataCallback;
    std::function<void(const unsigned char* data, unsigned int dataLen)> m_onDataCallback = nullptr; /* 数据回调 */
};
} // namespace npacket
