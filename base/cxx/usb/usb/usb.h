#pragma once
#include <libusb.h>
#include <memory>
#include <string>
#include <vector>

namespace usb
{
#ifdef _WIN32
struct WinUsb;
#endif

/**
 * @brief USB类
 */
class Usb
{
public:
    Usb();

    Usb(const Usb& src);

    virtual ~Usb() = default;

    /**
     * @brief 获取父节点
     * @return 父节点
     */
    std::shared_ptr<Usb> getParent() const;

    /**
     * @brief 获取总线编号
     * @return 总线编号
     */
    int getBusNum() const;

    /**
     * @brief 获取端口编号(Linux中也叫系统编号sysNum)
     * @return 端口编号
     */
    int getPortNum() const;

    /**
     * @brief 获取地址(每次拔插都会变)
     * @return 地址
     */
    int getAddress() const;

    /**
     * @brief 获取设备类型编码
     * @return 类型编码
     */
    int getClassCode() const;

    /**
     * @brief 获取设备类型编码(十六进制)
     * @return 类型编码
     */
    std::string getClassHex() const;

    /**
     * @brief 获取设备类型描述
     * @return 类型描述
     */
    std::string getClassDesc() const;

    /**
     * @brief 获取设备子类型编码
     * @return 子类型编码
     */
    int getSubClassCode() const;

    /**
     * @brief 获取设备协议编码
     * @return 协议编码
     */
    int getProtocolCode() const;

    /**
     * @brief 获取速度等级
     * @return 速度等级
     */
    int getSpeedLevel() const;

    /**
     * @brief 获取速度描述
     * @return 速度描述
     */
    std::string getSpeedDesc() const;

    /**
     * @brief 获取厂商ID
     * @return 厂商ID
     */
    std::string getVid() const;

    /**
     * @brief 获取产品ID
     * @return 产品ID
     */
    std::string getPid() const;

    /**
     * @brief 获取序列号
     * @return 序列号
     */
    std::string getSerial() const;

    /**
     * @brief 获取产品名称, 说明: 正常情况下可以根据是否包含`keyboard`或`mouse`来判断键盘/鼠标
     * @return 产品名称
     */
    std::string getProduct() const;

    /**
     * @brief 获取厂商名称
     * @return 厂商名称
     */
    std::string getManufacturer() const;

    /**
     * @brief 获取设备名称(Windows下才有值)
     * @return 设备名称
     */
    std::string getDeviceName() const;

    /**
     * @brief 获取设备描述(Windows下才有值)
     * @return 设备描述
     */
    std::string getDeviceDesc() const;

    /**
     * @brief 判断是否HID(键盘/鼠标/加密狗等)类型
     * @return true-是, false-否
     */
    bool isHid() const;

    /**
     * @brief 判断是否存储(U盘)类型
     * @return true-是, false-否
     */
    bool isStorage() const;

    /**
     * @brief 判断是否Hub(USB集线器)类型
     * @return true-是, false-否
     */
    bool isHub() const;

    /**
     * @brief 获取系统中USB设备列表
     * @param sf 是否获取序列号(选填), 默认不获取
     * @param pf 是否获取产品名称(选填), 默认不获取
     * @param mf 是否获取厂商名称(选填), 默认不获取
     * @return USB设备列表
     */
    static std::vector<Usb> getAllUsbs(bool sf = false, bool pf = false, bool mf = false);

#ifdef _WIN32
    /**
     * @brief 注册设备通知(用于拔插检测)
     *        如果是窗体程序, 则要监听: WM_DEVICECHANGE, 如果是服务程序, 则要监听: SERVICE_CONTROL_DEVICEEVENT
     *        插拔事件分别要监听: DBT_DEVICEARRIVAL, DBT_DEVICEREMOVECOMPLETE
     * @param handle 句柄
     * @return true-成功, false-失败
     */
    typedef void* HANDLE;
    static bool registerDeviceNotify(HANDLE handle);
#endif

private:
#ifdef _WIN32
    /**
     * @brief 利用Windows系统API获取WinUsb列表
     * @param winUsbList [输出]WinUsb列表
     */
    static void getWinUsbList(std::vector<WinUsb>& winUsbList);

    /**
     * @brief 递归判断WinUsb和Usb的父节点是否一致
     * @param winUsbList WinUsb列表
     * @param parentInstanceId WinUsb父节点实例ID
     * @param parent Usb父节点
     * @return true-一致(表示WinUsb和Usb节点互相匹配), false-不一致
     */
    static bool matchWinUsbParent(const std::vector<WinUsb>& winUsbList, std::string parentInstanceId, const std::shared_ptr<Usb>& parent);
#endif

    /**
     * @brief 解析得到Usb信息
     * @param dev 设备节点
     * @param sf 是否获取序列号
     * @param pf 是否获取产品名称
     * @param mf 是否获取厂商名称
     * @param winUsbList WinUsb列表(Windows平台下需要)
     * @param info [输出]Usb信息
     * @return true-成功, false-失败
     */
#ifdef _WIN32
    static bool parseUsb(libusb_device* dev, bool sf, bool pf, bool mf, const std::vector<WinUsb>& winUsbList, Usb& info);
#else
    static bool parseUsb(libusb_device* dev, bool sf, bool pf, bool mf, Usb& info);
#endif

private:
    std::shared_ptr<Usb> m_parent = nullptr; /* 父节点 */
    int m_busNum = 0; /* 总线编号 */
    int m_portNum = 0; /* 端口编号(Linux中也叫系统编号sysNum) */
    int m_address = 0; /* 地址(每次拔插都会变) */
    int m_classCode = 0; /* 设备类型编码(用于判断鼠标,键盘,Hub等) */
    int m_subClassCode = 0; /* 设备子类型编码 */
    int m_protocolCode = 0; /* 设备协议编码 */
    int m_speedLevel = 0; /* 速度等级 */
    std::string m_vid; /* 厂商ID(小写字母) */
    std::string m_pid; /* 产品ID(小写字母) */
    std::string m_serial; /* 序列号 */
    std::string m_product; /* 产品名称 */
    std::string m_manufacturer; /* 厂商名称 */
    std::string m_deviceName; /* 设备名称(Windows下才有值) */
    std::string m_deviceDesc; /* 设备描述(Windows下才有值) */
};
} // namespace usb
