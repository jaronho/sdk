#pragma once
#include <string>
#include <vector>

namespace usb
{
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
     * @brief 获取总线编号
     * @return 总线编号
     */
    int getBusNum() const;

    /**
     * @brief 获取端口编号(Linux中也叫系统编号)
     * @return 端口编号
     */
    int getPortNum() const;

    /**
     * @brief 获取地址
     * @return 地址
     */
    int getAddress() const;

    /**
     * @brief 获取设备类型编码
     * @return 类型编码
     */
    int getClassCode() const;

    /**
     * @brief 获取设备类型描述
     * @return 类型描述
     */
    std::string getClassDesc() const;

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
     * @brief 获取产品名称
     * @return 产品名称
     */
    std::string getProduct() const;

    /**
     * @brief 获取厂商名称
     * @return 厂商名称
     */
    std::string getManufacturer() const;

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

private:
    int m_busNum; /* 总线编号 */
    int m_portNum; /* 端口编号(Linux中也叫系统编号) */
    int m_address; /* 地址(每次拔插都会变) */
    int m_classCode; /* 设备类型编码(用于判断鼠标,键盘,Hub等) */
    std::string m_vid; /* 厂商ID */
    std::string m_pid; /* 产品ID */
    std::string m_serial; /* 序列号 */
    std::string m_product; /* 产品名称 */
    std::string m_manufacturer; /* 厂商名称 */
};
} // namespace usb
