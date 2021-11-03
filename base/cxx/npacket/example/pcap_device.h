#pragma once
#define _XKEYCHECK_H
#include <atomic>
#include <functional>
#include <memory>
#include <pcap.h>
#include <string>
#include <thread>
#include <vector>

class PcapDevice final
{
public:
    ~PcapDevice();

    /**
     * @brief 获取名字
     * @return 名字
     */
    std::string getName() const;

    /**
     * @brief 获取描述
     * @return 描述
     */
    std::string getDescribe() const;

    /**
     * @brief 获取IP地址
     * @return IP地址
     */
    std::string getAddress() const;

    /**
     * @brief 是否回环
     * @return true-是, false-否
     */
    bool isLoopback() const;

    /**
     * @brief 打开
     * @param snapLen 快照长度, 要捕获的数据包长度, 正常设置为65536能够满足所有网络
     * @param promisc 混杂模式, 该模式下适配器将接受所有数据包，即便那不是发到该适配器的, 0-普通模式, 1-混杂模式
     * @param timeout 超时读取, <=0表示永不超时
     * @param bufferSize 缓冲大小
     * @return true-成功, false-失败
     */
    bool open(int snapLen = 0, int promisc = 1, int timeout = 0, int bufferSize = 0);

    /**
     * @brief 设置数据回调
     * @param cb 数据回调, 参数: data-数据, dataLen-数据长度
     */
    void setDataCallback(const std::function<void(const unsigned char* data, int dataLen)>& cb);

    /**
     * @brief 开始捕获(非阻塞)
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
    std::string m_address; /* IP地址 */
    bool m_isLoopback = false; /* 是否回环 */
    pcap_t* m_pcap = nullptr; /* pcap指针 */
    std::thread* m_captureThread = nullptr; /* 数据包捕获线程 */
    std::atomic_bool m_captureStarted{false}; /* 是否已经开始捕获 */
    std::function<void(const unsigned char* data, int dataLen)> m_onDataCallback = nullptr; /* 数据回调 */
};