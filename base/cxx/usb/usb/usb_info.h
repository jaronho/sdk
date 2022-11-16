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
    /**
     * @brief 设备节点
     */
    struct DevNode
    {
        DevNode(const std::string& name, const std::string& fstype) : name(name), fstype(fstype) {}

        std::string name; /* 节点名, 如: /dev/sdb, /dev/sdb4, /dev/hidraw0 /dev/hidraw1 */
        std::string fstype; /* 文件系统类型, 如果是存储设备则为: exfat, vfat, ntfs等 */
    };

public:
    UsbInfo() = default;

    /**
     * @brief 构造函数
     * @param other 拷贝对象
     */
    UsbInfo(const Usb& other);

    /**
     * @brief 构造函数
     * @param other 拷贝对象
     * @param devNodes 设备节点列表
     */
    UsbInfo(const UsbInfo& other, const std::vector<DevNode>& devNodes);

    /**
     * @brief 判断是否相等
     * @param other 比较对象
     * @return true-相等, fasle-不相等
     */
    bool operator==(const UsbInfo& other) const;

    /**
     * @brief 判断是否不相等
     * @param other 比较对象
     * @return true-不相等, fasle-相等
     */
    bool operator!=(const UsbInfo& other) const;

    /**
     * @brief 获取节点列表
     * @return 设备节点列表
     */
    std::vector<DevNode> getDevNodes() const;

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
     * @param withDevNode 是否要查找设备节点(选填), 默认否, 当要挂载存储设备时要设置为true, 否则将无法挂载, 一般存储设备插入时系统
     *                    无法立即识别到其设备节点.
     *                    例如: 要对刚插入的U盘立即进行挂载, 由于系统需要一小段时间(可能几秒)后才能识别到U盘的设备节点, 此时
     *                    是无法进行挂载的, 可以设置为true, 然后循环调用该接口直到查到设备节点.
     * @param mf 是否要查询厂商名称(选填), 默认不查询
     * @return USB信息列表 
     */
    static std::vector<UsbInfo> queryUsbInfos(const std::function<bool(const UsbInfo& info)>& filterFunc, bool withDevNode = false,
                                              bool mf = false);

    /**
     * @brief 查询设备节点(Linux平台接口)
     * @param info USB信息
     * @return 设备节点列表 
     */
    static std::vector<DevNode> queryDevNodes(const UsbInfo& info);

private:
    std::vector<DevNode> m_devNodes; /* 节点(可能多个) */
};
} // namespace usb
