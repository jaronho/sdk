#pragma once
#ifdef _WIN32
#include <imapi2.h> /* 必须在<libusb.h>前包含 */
#endif
#include <functional>
#include <libusb.h>
#include <memory>
#include <string>
#include <vector>

namespace usb
{
/**
 * @brief 判断路径是否已设置为挂载点
 * @param path 路径
 * @return true-是, false-否
 */
bool isMountpoint(std::string path);

/**
 * @brief 光驱信息
 */
struct CdromInfo
{
    CdromInfo() = default;
    CdromInfo(const std::string& name) : name(name) {}

    std::string name; /* 设备名, 例如: /dev/sr0 */
#ifdef _WIN32
    int can_write_CD_R = 0; /* 是否支持写入CD-R光盘 */
    int can_write_CD_RW = 0; /* 是否支持写入CD-RW光盘 */
    int can_read_DVD = 0; /* 是否支持读取DVD光盘 */
    int can_write_DVD_R = 0; /* 是否支持写入DVD-R光盘 */
    int can_write_DVD_RAM = 0; /* 是否支持写入DVD-RAM光盘 */
#else
    int speed = 0; /* 读取速度, 例如: 4表示驱动器可以以4倍速读取CD */
    int slotNum = 0; /* 插槽数量, 对于大多数标准的单槽光驱, 这个值通常是1 */
    int can_close_tray = 0; /* 是否支持关闭托盘的能力 */
    int can_open_tray = 0; /* 是否支持打开托盘的能力 */
    int can_lock_tray = 0; /* 是否支持锁定托盘, 防止用户手动打开 */
    int can_change_speed = 0; /* 是否支持改变光驱的读取速度 */
    int can_select_disk = 0; /* 对于多盘位光驱, 是否支持在不同的盘之间切换, 则为1 */
    int can_read_multisession = 0; /* 是否支持读取多会话（Multi-Session）光盘 */
    int can_read_MCN = 0; /* 是否支持读取光盘上的媒体目录号（Media Catalog Number） */
    int reports_media_changed = 0; /* 是否支持在更换光盘后向系统报告媒体变更 */
    int can_play_audio = 0; /* 是否支持播放音频CD */
    int can_write_CD_R = 0; /* 是否支持写入CD-R光盘 */
    int can_write_CD_RW = 0; /* 是否支持写入CD-RW光盘 */
    int can_read_DVD = 0; /* 是否支持读取DVD光盘 */
    int can_write_DVD_R = 0; /* 是否支持写入DVD-R光盘 */
    int can_write_DVD_RAM = 0; /* 是否支持写入DVD-RAM光盘 */
    int can_read_MRW = 0; /* 是否支持读取MRW（Multi-Read/Multi-Write）格式的光盘 */
    int can_write_MRW = 0; /* 是否支持写入MRW（Multi-Read/Multi-Write）格式的光盘 */
    int can_write_RAM = 0; /* 是否支持写入CD-RAM（CD Random Access Memory）光盘 */
#endif
};

/**
 * @brief 获取光驱信息
 * @param outStr [输出]光驱原始信息(Linux平台)
 * @return 光驱信息
 */
#ifdef _WIN32
std::vector<CdromInfo> getCdromInfoList();
#else
std::vector<CdromInfo> getCdromInfoList(std::string& outStr);
#endif

/**
 * @brief 设备节点
 */
struct DevNode
{
    DevNode() = default;
    DevNode(const std::string& name, const std::string& pkname = "", const std::string& fstype = "", const std::string& label = "",
            const std::string& partlabel = "", size_t capacity = 0, const std::string& winDriver = "");

    /**
     * @brief 获取挂载点
     * @return 挂载点, 如: Windows: H:\, I:\, Linux: /mnt/myUdisk
     */
    std::string getMountpoint() const;

    std::string name; /* 节点名, 如, Linux: /dev/sdb1, /dev/sr0, /dev/hidraw0 等 */
    std::string pkname; /* 内部上级内核设备名称(可为空), 如, Linux: 如果节点名为/dev/sdb1, 则其上级名称为/dev/sdb */
    std::string fstype; /* 文件系统类型, 如果是存储设备则值为: ext4, vfat(FAT32), exfat(exFAT), ntfs(NTFS)等 */
    std::string label; /* 文件系统标签, 例如: "Jim's U-DISK" */
    std::string partlabel; /* 分区标签, 例如: "Microsoft reserved partition" */
    size_t capacity = 0; /* 磁盘容量 */

private:
    std::string m_winDriver; /* 设备驱动器(Windows平台), 如: H:\, I:\ */
};

struct UsbImpl;

/**
 * @brief USB类
 */
class Usb
{
public:
    Usb() = default;

    Usb(const Usb& src);

    virtual ~Usb() = default;

    /**
     * @brief 获取父节点
     * @return 父节点
     */
    std::shared_ptr<Usb> getParent() const;

    /**
     * @brief 获取子节点
     * @return 子节点
     */
    std::vector<std::weak_ptr<usb::Usb>> getChildren() const;

    /**
     * @brief 获取父路径
     * @return 路径, 例如: 总线_1级接口.2级接口.N级接口
     */
    std::string getParentPath() const;

    /**
     * @brief 获取路径
     * @return 路径, 例如: 总线_1级接口.2级接口.N级接口
     */
    std::string getPath() const;

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
     * @brief 获取设备子协议编码(如果设备为HID类型则值表示: 0-无协议, 1-键盘协议, 2-鼠标协议)
     * @return 子协议编码, 如果编码个数大于1表示该设备是复合设备
     */
    std::vector<int> getSubProtocolCode() const;

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
     * @brief 获取设备标识符(型号), 例如: "ELSKY_SSD_256GB", "CDRW_DVD_GCC4244", "DVD_A_DS8A5SH", "USB CARD READER " 等
     * @return 设备标识符
     */
    std::string getModel() const;

    /**
     * @brief 获取设备制造商, 例如: "FNK TECH", "HL-DT-ST", "Samsung " 等
     * @return 设备制造商
     */
    std::string getVendor() const;

    /**
     * @brief 获取存储设备类型, 值: disk-磁盘(机械盘,SSD,U盘,NVMe,SD卡等), part-分区, cdrom-光驱(CD,DVD,BD,ISO loop设备等)
     * @return 组名
     */
    std::string getStorageType() const;

    /**
     * @brief 获取节点列表(Windows一般至多1个, Linux平台可能多个)
     * @return 设备节点列表
     */
    std::vector<DevNode> getDevNodes() const;

    /**
     * @brief 获取光驱信息
     * @return 光驱信息
     */
    CdromInfo getCdromInfo() const;

    /**
     * @brief 判断是否HID(键盘/鼠标/加密狗等)类型
     * @return true-是, false-否
     */
    bool isHid() const;

    /**
     * @brief 判断是否存储设备类型
     * @return true-是, false-否
     */
    bool isStorage() const;

    /**
     * @brief 判断是否Hub(USB集线器)类型
     * @return true-是, false-否
     */
    bool isHub() const;

    /**
     * @brief 判断是否磁盘
     * @return true-是, false-否
     */
    bool isDisk() const;

    /**
     * @brief 判断是否光驱
     * @return true-是, false-否
     */
    bool isCdrom() const;

    /**
     * @brief 描述信息
     * @param showChildren 是否显示子节点
     * @param allIntend 全局缩进字符数
     * @param intend 内部缩进字符数
     */
    std::string describe(bool showChildren = false, int allIntend = 0, int intend = 4) const;

    /**
     * @brief 获取系统中USB设备列表
     * @param detailFlag 是否获取设备详情
     * @return USB设备列表
     */
    static std::vector<std::shared_ptr<usb::Usb>> getAllUsbs(bool detailFlag = true);

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
    /**
     * @brief 计算路径
     * @return 路径, 例如: 总线_1级接口.2级接口.N级接口
     */
    std::string calculatePath() const;

    /**
     * @brief 解析得到Usb信息
     * @param dev 设备节点
     * @param detailFlag 是否获取设备详情
     * @param implList UsbImpl列表
     * @param cdromList 光驱设备列表
     * @return Usb信息
     */
    static std::shared_ptr<usb::Usb> parseUsb(libusb_device* dev, bool detailFlag, const std::vector<UsbImpl>& implList,
                                              const std::vector<CdromInfo>& cdromList);

private:
    libusb_device* m_dev = NULL;
    std::shared_ptr<usb::Usb> m_parent = nullptr; /* 父节点 */
    std::vector<std::weak_ptr<usb::Usb>> m_children; /* 子节点(注意: 需要用弱引用否则会内存泄漏) */
    std::string m_parentPath; /* 父路径 */
    std::string m_path; /* 路径 */
    int m_busNum = -1; /* 总线编号 */
    int m_portNum = -1; /* 端口编号(Linux中也叫系统编号sysNum) */
    int m_address = -1; /* 地址(每次拔插都会变) */
    int m_classCode = -1; /* 设备类型编码(用于判断鼠标,键盘,Hub等) */
    int m_subClassCode = -1; /* 设备子类型编码 */
    int m_protocolCode = -1; /* 设备协议编码 */
    std::vector<int> m_subProtocolCode; /* 设备子协议编码, 如果设备为HID类型则值表示: 0-无协议, 1-键盘协议, 2-鼠标协议 */
    int m_speedLevel = -1; /* 速度等级 */
    std::string m_vid; /* 厂商ID(小写字母) */
    std::string m_pid; /* 产品ID(小写字母) */
    std::string m_serial; /* 序列号 */
    std::string m_product; /* 产品名称 */
    std::string m_manufacturer; /* 厂商名称 */
    std::string m_model; /* 设备标识符(型号), 例如: "ELSKY_SSD_256GB", "CDRW_DVD_GCC4244", "DVD_A_DS8A5SH", "USB CARD READER " 等 */
    std::string m_vendor; /* 设备制造商, 例如: "FNK TECH", "HL-DT-ST", "Samsung " 等 */
    std::string m_storageType; /* 存储类型, 值: disk-磁盘(机械盘,SSD,U盘,NVMe,SD卡等), part-分区, cdrom-光驱(CD,DVD,BD,ISO loop设备等) */
    std::vector<DevNode> m_devNodes; /* 设备节点(Windows一般至多1个, Linux平台可能多个) */
    CdromInfo m_cdromInfo; /* 光驱设备信息 */
};
} // namespace usb
