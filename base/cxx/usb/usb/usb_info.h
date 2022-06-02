#pragma once
#include <functional>

#include "usb.h"

namespace usb
{
/**
 * @brief USB信息
 */
class UsbInfo : public Usb
{
public:
    UsbInfo() = default;

    UsbInfo(const Usb& src);

    bool operator==(const UsbInfo& other) const;

    bool operator!=(const UsbInfo& other) const;

    /**
     * @brief 获取节点列表
     * @return 设备节点列表
     */
    std::vector<std::string> getDevNodes() const;

    /**
     * @brief 是否有效
     * @return true-有效, false-无效
     */
    bool isValid() const;

    /**
     * @brief 描述信息
     */
    std::string describe() const;

    /**
     * @brief 查询USB信息
     * @param filterFunc 过滤函数, 非空时不过滤, 参数: info-USB信息, 返回值: true-通过, false-被过滤
     * @param mustHaveDevNode 是否一定要包含设备节点(选填), 默认否, 为true时若设备节点未找到则忽略设备信息, 
     *                        当要挂载存储USB时要设置为true, 否则将无法挂载, 一般U盘插入时系统无法立即识别到其设备节点.
     *                        例如: 要对刚插入的U盘立即进行挂载, 由于系统需要一小段时间(可能几秒)后才能识别到U盘的设备节点,
     *                        此时是无法进行挂载的, 可以设置为true, 忽略找到的U盘设备信息, 然后循环调用该接口直到查到设备节点.
     * @param mf 是否要查询厂商名称(选填), 默认不查询
     * @return USB信息列表 
     */
    static std::vector<UsbInfo> queryUsbInfos(const std::function<bool(const UsbInfo& info)>& filterFunc, bool mustHaveDevNode = false,
                                              bool mf = false);

private:
    std::vector<std::string> m_devNodes; /* 节点(可能多个), 例如: /dev/sdb, /dev/sdb4, /dev/hidraw0 /dev/hidraw1 */
};
} // namespace usb
