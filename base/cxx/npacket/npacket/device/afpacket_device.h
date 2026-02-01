#pragma once
#ifdef __linux__

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace npacket
{
/**
 * @brief AF_PACKET设备(Linux平台高性能抓包)
 * 
 * 修复说明:
 * - 修复方向过滤功能(原PACKET_FANOUT误用)
 * - 修复字符串未终止导致的潜在越界
 * - 修复整数溢出和除零风险
 * - 优化锁粒度和V3处理效率
 */
class AfPacketDevice final
{
public:
    ~AfPacketDevice();

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
     * @brief 打开设备
     * @param name 设备名, 例如: eth0 (最大长度IFNAMSIZ-1)
     * @param direction 要捕获的数据流向, 0-所有, 1-仅接收, 2-仅发送(需要Linux 3.2+)
     * @param snapLen 快照长度, 要捕获的数据包长度, 正常设置为65536能够满足所有网络
     * @param promisc 混杂模式, 0-普通模式, 1-混杂模式
     * @param timeout 超时读取(毫秒), <=0表示永不超时
     * @param bufferSize 接收缓冲区大小(字节), >=8MB时自动启用TPACKET_V3
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
     * @param count 指定期望处理的最大数据包数量, 值: 0-无数量限制, >0-最多处理count个数据包
     * @return <0-错误, >=0-实际处理的数据包数量
     */
    int captureOnce(unsigned int count = 0);

    /**
     * @brief 开始捕获(启动后台线程)
     * @return true-成功, false-失败
     */
    bool startCapture();

    /**
     * @brief 停止捕获
     */
    void stopCapture();

    /**
     * @brief 关闭设备
     */
    void close();

    /**
     * @brief 获取本机所有AF_PACKET设备
     * @param errorBuffer [输出]错误信息
     * @return 设备列表
     */
    static std::vector<std::shared_ptr<AfPacketDevice>> getAllDevices(std::string* errorBuffer = nullptr);

private:
    /**
     * @brief 处理TPACKET_V3块
     * @return 处理的包数量
     */
    size_t processV3Block();

private:
    std::string m_name; /* 设备名 */
    std::string m_describe; /* 设备描述 */
    std::string m_ipv4Address; /* IPv4地址 */
    bool m_isLoopback = false; /* 是否回环 */
    std::recursive_mutex m_mutex; /* 设备操作互斥锁 */
    int m_sockfd = -1; /* AF_PACKET套接字描述符 */
    int m_snapLen = 65536; /* 快照长度 */
    int m_timeoutMs = 0; /* 超时时间(毫秒) */
    std::atomic<bool> m_captureStarted{false}; /* 是否已经开始捕获 */
    std::vector<uint8_t> m_buffer; /* 普通模式数据缓冲区 */
    std::mutex m_mutexOnDataCallback;
    std::function<void(const unsigned char* data, unsigned int dataLen)> m_onDataCallback = nullptr; /* 数据回调 */

    /* TPACKET_V3 相关成员 */
    bool m_useV3 = false; /* 是否使用TPACKET_V3 */
    uint8_t* m_ringBuffer = nullptr; /* 环形缓冲区映射地址 */
    size_t m_ringBufferSize = 0; /* 环形缓冲区大小 */
    struct tpacket_block_desc* m_currentBlock = nullptr; /* 当前块 */
    unsigned int m_blockIndex = 0; /* 当前块索引 */
    unsigned int m_blockCount = 0; /* 块总数 */
};
} // namespace npacket
#endif
