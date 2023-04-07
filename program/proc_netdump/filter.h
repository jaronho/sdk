#pragma once
#include <atomic>
#include <string>
#include <vector>

/**
 * @brief 过滤器
 */
class Filter final
{
private:
    Filter() = default;

public:
    /**
     * @brief 获取协议列表
     */
    static std::vector<std::string> protoList();

    /**
     * @brief 获取单例
     */
    static Filter& getInstance();

    /**
     * @brief 设置过滤条件
     */
    void setCondition(const std::string& condition);

    bool showEthernet() const;

    bool showIpv4() const;

    bool showArp() const;

    bool showIpv6() const;

    bool showTcp() const;

    bool showUdp() const;

    bool showIcmp() const;

    bool showIcmpv6() const;

    bool showFtp() const;

    bool showFtpData() const;

    bool showIec103() const;

private:
    std::atomic_bool m_showEthernet = {true};
    std::atomic_bool m_showIpv4 = {true};
    std::atomic_bool m_showArp = {true};
    std::atomic_bool m_showIpv6 = {true};
    std::atomic_bool m_showTcp = {true};
    std::atomic_bool m_showUdp = {true};
    std::atomic_bool m_showIcmp = {true};
    std::atomic_bool m_showIcmpv6 = {true};
    std::atomic_bool m_showFtp = {true};
    std::atomic_bool m_showFtpData = {true};
    std::atomic_bool m_showIec103 = {true};
};
