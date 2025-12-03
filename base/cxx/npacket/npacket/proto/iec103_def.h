#pragma once
#include <vector>

namespace npacket
{
namespace iec103
{
/**
 * @brief 数据单元标识
 */
struct DataUnitIdentify
{
    uint8_t type = 0; /* 类型标识 */
    struct /* 可变结构限定词 */
    {
        uint8_t continuous = 0; /* 信息体中元素的信息体地址是否连续: 0-断续,1-连续 */
        uint8_t num = 0; /* 信息体个数 */
    } vsq;
    uint8_t cot = 0; /* 传送原因 */
    uint8_t commonAddr = 0; /* 公共地址 */
};

/**
 * @brief 实际通道(Actural Channel), 取值(1-255)
 * 0 - 全局, 1 - IL1, 2 - IL2, 3 - IL3, 4 - IN, 5 - UL1E, 6 - UL2E, 7 - UL3E, 8 - UEN,
 * 9-63: 为将来兼容应用保留, 64-255: 为专用保留.
 */
typedef uint8_t ACC;

/**
 * @brief ASCII字符(ASCII character)
 */
typedef uint8_t ASC;

/**
 * @brief 兼容级别(Compatibity Level), 值[0,255], 未采用通用分类服务的从站兼容级别为2, 采用通用分类服务的兼容级别为3
 */
typedef uint8_t COL;

/**
 * @brief 双命令(Double Command), 0 - 未用, 1 - 跳(OFF), 2 - 合(ON), 3 - 未用
 */
typedef uint8_t DCO;

/**
 * @brief 双点信息(Double Point Information), 0 - 未用, 1 - 跳(OFF), 2 - 合(ON), 3 - 未用
 */
typedef uint8_t DPI;

/**
 * @brief 单点信息(Single Point Information)
 */
typedef uint8_t SPI;

/**
 * @brief 带瞬变和差错的双点信息
 */
typedef uint8_t TEDPI;

/**
 * @brief 故障序号(Fault Number), 值[0,65535], 用以识别合继电保护功能有关的一个事件, 例如:
 * 一次继电保护设备(或间隔单元)的启动/检出, 故障序号加1, 这意味着一次不成功的重合闸,
 * 将被记录成两次单独的故障序号, 故障序号不用复位或预置
 */
typedef uint16_t FAN;

/**
 * @brief 信息元素之间间隔(Interval Between Information Elements), 值[1,65535],
 * 对于全部扰动数据, 其单个信息元素采集的间隔是相同的, 它按微秒列表
 */
typedef uint16_t INT;

/**
 * @brief 带品质描述词的被测值(Measurand With Quality Descriptor)
 */
struct MEA
{
    uint8_t ov = 0; /* 溢出位, 0 - 无溢出, 1 - 溢出 */
    uint8_t er = 0; /* 差错位, 0 - 被测值(mval)有效, 1 - 被测值(mval)无效 */
    uint8_t res = 0; /* 备用位, 0 - 未用(常为0) */
    float mval = 0.0; /* 被测值, 若mval溢出时, 它被分别置成正或负的最大值, 而且当ov=1, 最大的mval可以是+/-1.2或+/-2.4额定值 */
};

/**
 * @brief 应用服务数据单元的第一个信息元素的序号(Number of the ASUD's First Information Element), 值[0,65535]
 */
typedef uint16_t NFE;

/**
 * @brief 通道数目(Number Of Channels), 值[0,255], 指明准备传输的一组数据集的模拟(类似)通道数目
 */
typedef uint8_t NOC;

/**
 * @brief 一个通道信息元素的数目(Number Of Information Element Of A Channel), 值[1,65535],
 * 用于ASDU26: 扰动数据传输准备就绪
 */
typedef uint16_t NOE;

/**
 * @brief 电网故障序号(Number Of Grid Faults), 值[0,65535], 一次电网故障(即一次短路), 可能因跳闸合自动重合闸而引
 * 起好几次故障, 每次故障由故障序号加1而加以识别, 而此时, 电网故障序号仍保持不变, 电网故障序号不需要复位和预置
 */
typedef uint16_t NOF;

/**
 * @brief 带标志的状态变位数目(Number Of Tags), 值[1,255], 表示每一个应用服务数据单元(ASDU)传送的带标志的状态变位数目
 */
typedef uint8_t NOT;

/**
 * @brief 每个应用服务数据单元有关联扰动值的数目(Number Of Relevant Disturbance Values Per ASDU), 1-25 - 使用, 26-255 - 未用
 */
typedef uint8_t NDV;

/**
 * @brief 相对时间(Relative Time), 值[0,65535], 在短路开始时将相对时间复位, 
 * 它指出继电保护设备(或间隔单元)从启动/检出故障到现在的时间, 它用毫秒表示
 */
typedef uint16_t RET;

/**
 * @brief 参比因子(Reference Factor)
 */
typedef double RFA;

/**
 * @brief 额定一次值(Rated Primary Value)
 */
typedef double RPV;

/**
 * @brief 额定二次值(Rated Secondary Value)
 */
typedef double RSV;

/**
 * @brief 返回信息标识符(Return Information Identifier), 值[0,255]
 */
typedef uint8_t RII;

/**
     * @brief 短路位置(Short Circuit Location), 用相对一次值得故障电抗来代表, 用欧姆表示
     */
typedef double SCL;

/**
 * @brief 扫描序号(Scan Number), 值[0,255]
 */
typedef uint8_t SCN;

/**
 * @brief 单个扰动值(Single Disturbance Value)
 */
typedef float SDV;

/**
 * @brief 附加信息(Supplementary Information), 值[0,255], 使用如下:
 * 当传送原因(COT)=总查询(总召唤)时, SIN=总查询(总召唤-GI)启动得应用服务数据单元的扫描序号;
 * 当传送原因(COT)=命令认可(肯定或否定)时, SIN=命令报文的返回信息标识符(RII);
 * 当传送原因(COT)=其他时, SIN=无关
 */
typedef uint8_t SIN;

/**
 * @brief 故障的状态(Status Of Fault)
 */
struct SOF
{
    uint8_t tp = 0; /* 0 - 被记录的故障未跳闸, 1 - 被记录的故障跳闸 */
    uint8_t tm = 0; /* 0 - 扰动数据等待传输, 1 - 扰动数据正在传输 */
    uint8_t test = 0; /* 0 - 在正常操作时被记录的扰动数据, 1 - 在测试模式下被记录的扰动数据 */
    uint8_t otev = 0; /* 0 - 由启动/检出故障触发被记录的扰动数据, 1 - 由其他事件触发被记录的扰动数据 */
    uint8_t res = 0; /* 备用(未用) */
};

/**
 * @brief 带标志的状态变位的位置(Tag Position), 值[0,65505], 表示带标志的状态变位在扰动数据集内的位置, 此数目指明带标志
 * 的状态变位和扰动数据集的第一个元素的距离, 并按信息元素(TOO)数据的65536模编码, 第一个带标志的状态变位的位置为0
 */
typedef int16_t TAP;

/**
 * @brief 命令类型(Type Of Order), 值[1,255]
 * 1 - 故障的选择, 2 - 请求扰动数据, 3 - 中止扰动数据, 4-7 - 备用, 8 - 通道的请求, 9 - 通道的中止, 10-15 - 备用,
 * 16 - 请求带标志的状态变位, 17 - 中止带标志的状态变位, 18-23 - 备用, 24 - 请求被记录扰动表, 25-31 - 备用,
 * 32 - 不带中止的扰动数据传输的结束, 33 - 由控制系统所中止的扰动数据传输的结束, 34 - 由继电保护设备(或间隔单元)所中止的扰动数据传输的结束
 * 35 - 不带中止的通道传输的结束, 36 - 由控制系统所中止的通道传输的结束, 37 - 由继电保护设备(或间隔单元)所中止的通道传输的结束
 * 38 - 不带中止的带标志的状态变位的传输的结束, 39 - 由控制系统所中止的带标志的状态变位的传输的结束
 * 40 - 由继电保护设备(或间隔单元)所中止的带标志的状态变位传输的结束, 41-63 - 备用, 64 - 成功地扰动数据传输(肯定)
 * 65 - 不成功地扰动数据传输(否定), 66 - 成功地通道传输(肯定), 67 - 不成功地通道传输(否定)
 * 68 - 成功地带标志的状态变位传输(肯定), 69 - 不成功地带标志的状态变位传输(否定), 70-255 - 备用.
 * 值(1-31)用于ASDU24：扰动数据传输的命令;
 * 值(32-63)用于ASDU31：扰动数据传输的结束;
 * 值(64-95)用于 ASDU25：扰动数据传输的认可
 */
typedef uint8_t TOO;

/**
 * @brief 扰动值的类型(Type Of Disturbance Values), 0 - 未用, 1 - 瞬时值, 2-255 - 未用
 */
typedef uint8_t TOV;

/**
 * @brief 四个八位位组的二进制时间(Four Octet Binary Time), 参考: CP56Time2a
 */
struct CP32Time2a
{
    uint8_t hour = 0; /* 时 */
    uint8_t minute = 0; /* 分 */
    uint16_t millisecond = 0; /* 毫秒 */
    uint8_t summerTime = 0; /* 夏时制 */
};

/**
 * @brief 七个八位位组的二进制时间(Seven Octet Binary Time)
 * ---------------------------------------------------------
 * |  D7  |  D6  |  D5  |  D4  |  D3  |  D2  |  D1  |  D0  |
 * |-------------------------------------------------------|
 * |                       毫秒(低8位)                     |
 * |-------------------------------------------------------|
 * |                       毫秒(高8位)                     |
 * |-------------------------------------------------------|
 * | 无效 | 保留 |                   分钟                  |
 * |-------------------------------------------------------|
 * |  SU  |     保留    |               小时               |
 * |-------------------------------------------------------|
 * |     星期的某天     |           一个月的某天           |
 * |-------------------------------------------------------|
 * |             保留          |             月            |
 * |-------------------------------------------------------|
 * | 保留 |                        年                      |
 * ---------------------------------------------------------
 * 例如: E3 A1 1F 0A 51 09 17
 * 前2个字节 E3 A1 表示毫秒, 值计算: (A1 << 8) | E3 = 41443(毫秒);
 * 第3个字节 1F(00011111) 的第8位表示: 0 - 有效, 1 - 无效, 第7位保留, 低6位表示分钟, 值计算: 1F & 0x3f = 31(分钟);
 * 第4个字节 0A(00001010) 的第8位表示是否夏时制, 第7和第6位保留, 低5位表示小时, 值计算: 0A & 0x1f = 10(小时);
 * 第5个字节 51(01010001) 的高4位表示星期的某天, 值计算: 51 & 0xe0 = 5(星期几, 0-星期天), 低4位表示月的某天, 值计算: 51 & 0x1f = 1(日);
 * 第6个字节 09(00001001) 的高4位保留, 低4位表示月, 值计算: 09 & 0x0f = 9(月);
 * 第7个字节 16(00010110) 的第8位保留, 剩余7位表示年, 值计算: 16 & 0x7f = 22, 如果值<70则加上2000, 如果值>=70则加上1900, 得出2022(年);
 * 最终日期为: 2022-09-01 10:31:41 星期四
 */
struct CP56Time2a
{
    uint16_t year = 0; /* 年 */
    uint8_t month = 0; /* 月 */
    uint8_t day = 0; /* 日 */
    uint8_t wday = 0; /* 星期几, 0-星期天 */
    uint8_t hour = 0; /* 时 */
    uint8_t minute = 0; /* 分 */
    uint16_t millisecond = 0; /* 毫秒 */
    uint8_t summerTime = 0; /* 夏时制 */
};

/**
 * @brief 通用分类数据集数目(Number Of Generic Data Sets)
 */
struct NGD
{
    uint16_t no = 0; /* 数目, 值[1,63] */
    uint8_t count = 0; /* 计数器位, 值[0,1], 具有相同返回信息标识符(RII)的应用服务数据单元的一位计算器位 */
    uint8_t cont = 0; /* 后续状态位, 0 - 后面未跟着具有相同返回信息标识符(RII)的应用服务数据单元, 1 - 跟着 */
};

/**
 * @brief 通用分类标识序号(Generic Identification Number)
 */
struct GIN
{
    uint8_t group = 0; /* 组, 值[0,255] */
    uint8_t entry = 0; /* 条目, 值, 0 - 组标识符, 1-255 - 条目标识符 */
};

/**
 * @brief 通用分类数据描述(Generic Data Description)
 */
struct GDD
{
    /**
     * 数据类型, 0 - 瞬变, 1 - 开, 2 - 合, 3 - 差错, 12 - 带品质描述词的被测值, 13 - 备用, 14 - 二进制时间,
     * 15 - 通用分类标识序号, 16 - 相对时间, 17 - 功能类型和信息序号, 18 - 带时标的报文, 19 - 带相对时间的时标报文,
     * 20 - 带相对时间的时标的被测值, 21 - 外部文本序号, 22 - 通用分类回答码, 23 - 数据结构, 24 - 索引, 25-255 - 备用
     */
    uint8_t dataType = 0;
    uint8_t dataSize = 0; /* 数据宽度, 值[1,255] */
    uint8_t number = 0; /* 数目, 值[1,127] */
    uint8_t cont = 0; /* 后续状态位, 0 - 后面未跟随数据元素, 1 - 后面跟随的应用服务数据单元的数据中具有相同的返回信息标识符(RII) */
};

/**
 * @brief 描述的类别(Kind Of Description), 值[0,255]
 * 0 - 无所指定的描述类别, 1 - 实际值, 2 - 缺省值, 3 - 量程, 4 - 备用, 5 - 精度, 6 - 因子, 7 - %参比,
 * 8 - 列表, 9 - 量纲, 10 - 描述, 11 - 备用, 12 - 口令条目, 13 - 只读, 14 - 只写, 15 - 备用, 16 - 备用,
 * 17 - 备用, 18 - 备用, 19 - 相应的功能类型和信号序号, 20 - 相应的事件, 21 - 列表的文本阵列,
 * 22 - 列表的值阵列, 23 - 相关联的条目, 24-255 - 备用
 */
typedef uint8_t KOD;

/**
 * @brief 描述元素的数目(Number Of Description Elements)
 */
struct NDE
{
    uint16_t no = 0; /* 数目, 值[1,63] */
    uint8_t count = 0; /* 计数器位 */
    uint8_t cont = 0; /* 后续位, 0 - 后面未跟随具有相同返回标识符(RII)和相同的通用分类标识序号(GIN)的应用服务数据单元, 1 - 跟着 */
};

/**
 * @brief 通用分类回答码(Generic Reply Code)
 * 0 - 认可, 1 - 无效的通用分类标识序号, 2 - 不存在所请求的数据, 3 - 数据不能用(过后再来一次), 4 - 改变设定时确认出错,
 * 5 - 改变设定时超出量程, 6 - 条目的范围太大, 7 - 太多的命令, 8 - 条目是只读, 9 - 设定受口令保护, 10 - 当地设定在进行中,
 * 11 - 带有下面所描述的差错, 12-255 - 未用
 */
typedef uint8_t GRC;

/**
 * @brief 通用分类标识数目(Number Of Generic Identification), 值[0,255]
 */
typedef uint8_t NOG;

/**
 * @brief 带时标的报文
 */
struct DPIWithTime
{
    DPI dpi = 0; /* 双点信息 */
    CP32Time2a tm; /* 时间 */
    SIN sin = 0; /* 附加信息 */
};

/**
 * @brief 带相对时间的时标报文
 */
struct DPIWithRet
{
    DPI dpi = 0; /* 双点信息 */
    RET ret = 0; /* 相对时间 */
    FAN fan = 0; /* 故障序号 */
    CP32Time2a tm; /* 时间 */
    SIN sin = 0; /* 附加信息 */
};

/**
 * @brief 带相对时间的时标的被测值
 */
struct ValWithRet
{
    float real32Value = 0.0; /* 7. IEEE标准754短实数 */
    RET ret = 0; /* 相对时间 */
    FAN fan = 0; /* 故障序号 */
    CP32Time2a tm; /* 时间 */
};

/**
 * @brief 通用分类标识数据(Generic Identification Data)
 * 说明: 根据GDD的数据类型使用不同的数据
 */
struct GID
{
    ASC ascii = 0; /* 1. ASCII字符 */
    uint8_t bsi[4] = {0}; /* 2. 成组8位串 */
    uint32_t uintValue = 0; /* 3. 无符号整数 */
    int32_t intValue = 0; /* 4. 整数 */
    double urealValue = 0.0; /* 5. 无符号浮点数 */
    float realValue = 0.0; /* 6. 浮点数 */
    float real32Value = 0.0; /* 7. IEEE标准754短实数 */
    double real64Value = 0.0; /* 8. IEEE标准754实数 */
    DPI dpi = 0; /* 9. 双点信息 */
    uint8_t spi = 0; /* 10. 单点信息 */
    uint8_t tedpi = 0; /* 11. 带瞬变和差错的双点信息 */
    MEA mea; /* 12. 带品质描述词的被测值 */
    /* 13. 备用 */
    CP56Time2a tm; /* 14. 七个八位位组的二进制时间 */
    GIN gin; /* 15. 通用分类标识序号 */
    RET ret = 0; /* 16. 相对时间 */
    uint8_t func = 0; /* 17. 功能类型 */
    uint8_t inf = 0; /* 17. 信号序列 */
    DPIWithTime dpiWithTime; /* 18. 带时标的报文 */
    DPIWithRet dpiWithRet; /* 19. 带相对时间的时标报文 */
    ValWithRet valWithRet; /* 20. 带相对时间的时标的被测值 */
    /* 21. 外部文本序号 */
    GRC grc = 0; /* 22. 通用分类回答码 */
    GDD gdd; /* 23. 数据结构 */
    /* 24. 索引, 25-255 - 备用 */
};

/**
 * @brief 带瞬变状态指示的值
 */
struct VTI
{
    uint8_t type = 0; /* 0 - 设备未在瞬变状态, 1 - 设备在瞬变状态 */
    uint8_t value = 0; /* 分接头位置 */
};

/**
 * @brief 品质描述
 */
struct QDS
{
    uint8_t iv = 0;
    uint8_t nt = 0;
    uint8_t sb = 0;
    uint8_t bl = 0;
    uint8_t res = 0;
    uint8_t ov = 0;
};

/**
 * @brief 应用层服务数据单元
 */
class Asdu
{
public:
    DataUnitIdentify identify; /* 数据单元标识 */
};

class Asdu1 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    DPI dpi = 0; /* 双点信息 */
    CP32Time2a tm; /* 时间 */
    SIN sin = 0; /* 附加信息 */
};

class Asdu2 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    DPI dpi = 0; /* 双点信息 */
    RET ret = 0; /* 相对时间 */
    FAN fan = 0; /* 故障序号 */
    CP32Time2a tm; /* 时间 */
    SIN sin = 0; /* 附加信息 */
};

class Asdu3 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    std::vector<MEA> meaList; /* 带品质描述值的被测值, 从0开始依次为: B相电流, AB线电压, 有功功率, 无功功率 */
};

class Asdu4 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    SCL scl = 0.0; /* 短路位置 */
    RET ret = 0; /* 相对时间 */
    FAN fan = 0; /* 故障序号 */
    CP32Time2a tm; /* 时间 */
};

class Asdu5 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    COL col = 0; /* 兼容级别 */
    ASC ascii1 = 0; /* ASCII字符1 */
    ASC ascii2 = 0; /* ASCII字符2 */
    ASC ascii3 = 0; /* ASCII字符3 */
    ASC ascii4 = 0; /* ASCII字符4 */
    ASC ascii5 = 0; /* ASCII字符5 */
    ASC ascii6 = 0; /* ASCII字符6 */
    ASC ascii7 = 0; /* ASCII字符7 */
    ASC ascii8 = 0; /* ASCII字符8 */
    uint8_t freeValue1 = 0; /* 可由制造厂自由赋值1 */
    uint8_t freeValue2 = 0; /* 可由制造厂自由赋值2 */
    uint8_t freeValue3 = 0; /* 可由制造厂自由赋值3 */
    uint8_t freeValue4 = 0; /* 可由制造厂自由赋值4 */
};

class Asdu6 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    CP56Time2a tm; /* 时间 */
};

class Asdu7 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    SCN scn = 0; /* 扫描序号 */
};

class Asdu8 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    SCN scn = 0; /* 扫描序号 */
};

class Asdu9 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    /* 带品质描述值的被测值, 从0开始依次为: A相电流, B相电流, C相电流, A相电压, B相电压, C相电压, 三相有功功率, 三相无功功率, (频率, 偏移50Hz的差值范围-2.000~2.000Hz) */
    std::vector<MEA> meaList;
};

struct DataSet10
{
    GIN gin; /* 通用分类标识序号 */
    KOD kod = 0; /* 描述类别 */
    GDD gdd; /* 通用分类数据描述 */
    std::vector<GID> gidList; /* 通用分类标识数据集 */
};

class Asdu10 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    RII rii = 0; /* 返回信息标识符 */
    NGD ngd; /* 通用分类数据集数目 */
    std::vector<DataSet10> dataSet; /* 数据集 */
};

struct DataSet11
{
    KOD kod = 0; /* 描述类别 */
    GDD gdd; /* 通用分类数据描述 */
    std::vector<GID> gidList; /* 通用分类标识数据集 */
};

class Asdu11 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    RII rii = 0; /* 返回信息标识符 */
    GIN gin; /* 通用分类标识序号 */
    NDE nde; /* 描述元素的数目 */
    std::vector<DataSet11> dataSet; /* 数据集 */
};

class Asdu20 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    DCO dco = 0; /* 双命令 */
    RII rii = 0; /* 返回信息序号 */
};

struct DataSet21
{
    GIN gin; /* 通用分类标识序号 */
    KOD kod = 0; /* 描述类别 */
};

class Asdu21 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信号序列 */
    RII rii = 0; /* 返回信息序号 */
    NOG nog = 0; /* 通用分类标识数目 */
    std::vector<DataSet21> dataSet; /* 数据集 */
};

struct DataSet23
{
    FAN fan = 0; /* 故障序号 */
    SOF sof; /* 故障状态 */
    CP56Time2a tm; /* 时间 */
};

class Asdu23 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t _ = 0; /* 未用(固定为0) */
    std::vector<DataSet23> dataSet; /* 数据集 */
};

class Asdu24 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t _ = 0; /* 未用(固定为0) */
    TOO too = 0; /* 命令类型 */
    TOV tov = 0; /* 扰动值的类型 */
    FAN fan = 0; /* 故障序号 */
    ACC acc = 0; /* 实际通道序号 */
};

class Asdu25 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t _ = 0; /* 未用(固定为0) */
    TOO too = 0; /* 命令类型 */
    TOV tov = 0; /* 扰动值的类型 */
    FAN fan = 0; /* 故障序号 */
    ACC acc = 0; /* 实际通道序号 */
};

class Asdu26 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t _ = 0; /* 未用(固定为0) */
    uint8_t __ = 0; /* 未用(固定为0) */
    TOV tov = 0; /* 扰动值的类型 */
    FAN fan = 0; /* 故障序号 */
    NOF nof = 0; /* 电网故障序号 */
    NOC noc = 0; /* 通道数目 */
    NOE noe = 0; /* 一个通道信息元素的数目 */
    INT interval = 0; /* 信息元素之间间隔 */
    CP32Time2a tm; /* 时间 */
};

class Asdu27 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t _ = 0; /* 未用(固定为0) */
    uint8_t __ = 0; /* 未用(固定为0) */
    TOV tov = 0; /* 扰动值的类型 */
    FAN fan = 0; /* 故障序号 */
    ACC acc = 0; /* 实际通道 */
    RPV rpv = 0.0; /* 额定一次值 */
    RSV rsv = 0.0; /* 额定二次值 */
    RFA rfa = 0.0; /* 参比因子 */
};

class Asdu28 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t _ = 0; /* 未用(固定为0) */
    uint8_t __ = 0; /* 未用(固定为0) */
    uint8_t ___ = 0; /* 未用(固定为0) */
    FAN fan = 0; /* 故障序号 */
};

struct DataSet29
{
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信息序号 */
    DPI dpi = 0; /* 双点信息 */
};

class Asdu29 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t _ = 0; /* 未用(固定为0) */
    FAN fan = 0; /* 故障序号 */
    NOT not_ = 0; /* 带标志的状态变位数目 */
    TAP tap = 0; /* 带标志的状态变位的位置 */
    std::vector<DataSet29> dataSet; /* 数据集 */
};

class Asdu30 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t _ = 0; /* 未用(固定为0) */
    uint8_t __ = 0; /* 未用(固定为0) */
    TOV tov = 0; /* 扰动值类型 */
    FAN fan = 0; /* 故障序号 */
    ACC acc = 0; /* 实际通道 */
    NDV ndv = 0; /* 扰动值数目 */
    NFE nfe = 0; /* 应用服务数据单元的第一个信息元素的序号 */
    std::vector<SDV> sdvList; /* 扰动值列表 */
};

class Asdu31 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t _ = 0; /* 未用(固定为0) */
    TOO too = 0; /* 命令类型 */
    TOV tov = 0; /* 扰动值类型 */
    FAN fan = 0; /* 故障序号 */
    ACC acc = 0; /* 实际通道 */
};

class Asdu38 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信息序号 */
    VTI vti; /* 带瞬变状态指示的值 */
    QDS qds; /* 品质描述 */
};

class Asdu42 final : public Asdu
{
public:
    uint8_t func = 0; /* 功能类型 */
    uint8_t inf = 0; /* 信息序号 */
    std::vector<DPI> dpiList; /* 双点信息列表 */
    SIN sin = 0; /* 附加信息 */
};

/**
 * @brief 固定帧 
 */
struct FixedFrame
{
    int prm = 0; /* 启动报文位, 1 - 主站向从站传输, 0 - 从站向主站传输 */
    int fcb_acd = 0; /* 当prm为1时表示fcb(帧计数位), 当prm为0时表示acd(要求访问位) */
    int fcv_dfc = 0; /* 当prm为1时表示fcv(帧计数有效位), 当prm为0时表示dfc(数据流控制位) */
    int func = 0; /* 功能码 */
    int addr = 0; /* 地址域 */
};

/**
 * @brief 可变帧 
 */
struct VariableFrame
{
    int prm = 0; /* 启动报文位, 1 - 主站向从站传输, 0 - 从站向主站传输 */
    int fcb_acd = 0; /* 当prm为1时表示fcb(帧计数位), 当prm为0时表示acd(要求访问位) */
    int fcv_dfc = 0; /* 当prm为1时表示fcv(帧计数有效位), 当prm为0时表示dfc(数据流控制位) */
    int func = 0; /* 功能码 */
    int addr = 0; /* 地址域 */
    std::shared_ptr<Asdu> asdu = nullptr; /* 应用服务数据单元 */
};
} // namespace iec103
} // namespace npacket
