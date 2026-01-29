#pragma once
#include <memory>
#include <stdint.h>

namespace npacket
{
namespace modbus
{
constexpr uint32_t MAX_PDU_LENGTH = 253; /* Modbus协议最大PDU长度 */
constexpr uint32_t RTU_MIN_FRAME = 4; /* Modbus/RTU最小帧长度: 地址(1) + 功能码(1) + CRC(2) = 4字节 */
constexpr uint32_t MBAP_HEADER_LEN = 7; /* Modbus/TCP MBAP头固定长度: 事务ID(2) + 协议ID(2) + 长度(2) + 单元ID(1) = 7字节 */

/**
 * @brief 功能码
 */
enum FunctionCode
{
    /* 基础功能码 */
    READ_COILS = 0x01, /* 读取线圈状态(0x0000-0xFFFF) */
    READ_DISCRETE_INPUTS = 0x02, /* 读取离散输入状态(0x0000-0xFFFF) */
    READ_HOLDING_REGISTERS = 0x03, /* 读取保持寄存器(0x0000-0xFFFF) */
    READ_INPUT_REGISTERS = 0x04, /* 读取输入寄存器(0x0000-0xFFFF) */
    WRITE_SINGLE_COIL = 0x05, /* 写单个线圈(0x0000-0xFFFF) */
    WRITE_SINGLE_REGISTER = 0x06, /* 写单个寄存器(0x0000-0xFFFF) */
    READ_EXCEPTION_STATUS = 0x07, /* 读取8个异常输出线圈状态(无请求数据) */

    /* 诊断功能码 */
    DIAGNOSTICS = 0x08, /* 诊断功能(子功能码0x0000-0xFFFF) */
    GET_COM_EVENT_COUNTER = 0x0B, /* 获取通信事件计数器(无请求数据) */
    GET_COM_EVENT_LOG = 0x0C, /* 获取通信事件日志(无请求数据) */
    REPORT_SERVER_ID = 0x11, /* 报告从站ID(无请求数据) */

    /* 高级功能码 */
    WRITE_MULTIPLE_COILS = 0x0F, /* 写多个线圈(0x0000-0xFFFF) */
    WRITE_MULTIPLE_REGISTERS = 0x10, /* 写多个寄存器(0x0000-0xFFFF) */
    MASK_WRITE_REGISTER = 0x16, /* 屏蔽写寄存器(0x0000-0xFFFF) */
    READ_WRITE_MULTIPLE_REGISTERS = 0x17, /* 读写多个寄存器(0x0000-0xFFFF) */
    READ_FIFO_QUEUE = 0x18, /* 读FIFO队列(0x0000-0xFFFF) */

    /* 文件记录 */
    READ_FILE_RECORD = 0x14, /* 读文件记录(0x0000-0xFFFF) */
    WRITE_FILE_RECORD = 0x15, /* 写文件记录(0x0000-0xFFFF) */

    /* 封装接口 */
    READ_DEVICE_IDENTIFICATION = 0x2B, /* 读取设备标识(0x2B/0x0E) */

    /* 异常响应标记 */
    EXCEPTION_MASK = 0x80 /* 异常响应标志位 */
};

/**
 * @brief 异常响应码
 */
enum ExceptionCode
{
    ILLEGAL_FUNCTION = 0x01, /* 不合法功能码 */
    ILLEGAL_DATA_ADDRESS = 0x02, /* 不合法数据地址 */
    ILLEGAL_DATA_VALUE = 0x03, /* 不合法数据值 */
    SERVER_DEVICE_FAILURE = 0x04, /* 从站设备故障 */
    ACKNOWLEDGE = 0x05, /* 应答(用于编程模式) */
    SERVER_DEVICE_BUSY = 0x06, /* 从站设备忙 */
    MEMORY_PARITY_ERROR = 0x08, /* 存储器奇偶校验错误 */
    GATEWAY_PATH_UNAVAILABLE = 0x0A, /* 网关路径不可用 */
    GATEWAY_TARGET_FAILED_TO_RESPOND = 0x0B /* 网关目标设备响应失败 */
};

/**
 * @brief 已解析出来的数据
 */
struct FuncDataSt
{
    /**
     * @brief 是否为请求
     * @return true-请求包, false-响应包
     */
    virtual bool isRequest() const = 0;
};

/**
 * @brief 读取线圈请求, 主站→从站, PDU: 起始地址(2字节) + 数量(2字节)
 */
struct ReadCoilsRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t startAddress = 0; /* 起始地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t quantity = 0; /* 读取线圈数量(1-2000, 大端字节序) */
};

/**
 * @brief 读取线圈响应, 从站→主站, PDU: 字节计数(1字节) + 线圈状态(N字节)
 */
struct ReadCoilsResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint8_t byteCount = 0; /* 后续状态数据的字节数(N = (quantity+7)/8) */
    const uint8_t* status = nullptr; /* 位状态数据(LSB优先, 每个bit代表一个线圈 */
};

/**
 * @brief 读取离散输入请求, 主站→从站, PDU: 起始地址(2字节) + 数量(2字节)
 */
struct ReadDiscreteInputsRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t startAddress = 0; /* 起始地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t quantity = 0; /* 读取输入数量(1-2000, 大端字节序) */
};

/**
 * @brief 读取离散输入响应, 从站→主站, PDU: 字节计数(1字节) + 输入状态(N字节)
 */
struct ReadDiscreteInputsResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint8_t byteCount = 0; /* 后续状态数据的字节数(N = (quantity+7)/8) */
    const uint8_t* status = nullptr; /* 位状态数据(LSB优先, 每个bit代表一个输入 */
};

/**
 * @brief 读取保持寄存器请求, 主站→从站, PDU: 起始地址(2字节) + 数量(2字节)
 */
struct ReadHoldingRegistersRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t startAddress = 0; /* 起始地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t quantity = 0; /* 读取寄存器数量(1-125, 大端字节序) */
};

/**
 * @brief 读取保持寄存器响应, 从站→主站, PDU: 字节计数(1字节) + 寄存器值(N×2字节)
 */
struct ReadHoldingRegistersResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint8_t byteCount = 0; /* 后续数据的字节数(N = quantity×2) */
    const uint16_t* values = nullptr; /* 寄存器值数组(每个寄存器2字节, 大端字节序) */
};

/**
 * @brief 读取输入寄存器请求, 主站→从站, PDU: 起始地址(2字节) + 数量(2字节)
 */
struct ReadInputRegistersRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t startAddress = 0; /* 起始地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t quantity = 0; /* 读取寄存器数量(1-125, 大端字节序) */
};

/**
 * @brief 读取输入寄存器响应, 从站→主站, PDU: 字节计数(1字节) + 寄存器值(N×2字节)
 */
struct ReadInputRegistersResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint8_t byteCount = 0; /* 后续数据的字节数(N = quantity×2) */
    const uint16_t* values = nullptr; /* 寄存器值数组(每个寄存器2字节, 大端字节序) */
};

/**
 * @brief 写单个线圈请求, 主站→从站, PDU: 地址(2字节) + 值(2字节, 0xFF00=ON, 0x0000=OFF)
 */
struct WriteSingleCoilRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t address = 0; /* 线圈地址(0x0000-0xFFFF, 大端字节序) */
    bool value = false; /* 写入值(true=ON(0xFF00), false=OFF(0x0000)) */
};

/**
 * @brief 写单个线圈响应, 从站→主站, PDU: 地址(2字节) + 值(2字节, 0xFF00=ON, 0x0000=OFF)
 */
struct WriteSingleCoilResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint16_t address = 0; /* 线圈地址(0x0000-0xFFFF, 大端字节序) */
    bool value = false; /* 写入值(true=ON(0xFF00), false=OFF(0x0000)) */
};

/**
 * @brief 写单个寄存器请求, 主站→从站, PDU: 地址(2字节) + 值(2字节)
 */
struct WriteSingleRegisterRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t address = 0; /* 寄存器地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t value = 0; /* 写入值(0x0000-0xFFFF, 大端字节序) */
};

/**
 * @brief 写单个寄存器响应, 从站→主站, PDU: 地址(2字节) + 值(2字节)
 */
struct WriteSingleRegisterResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint16_t address = 0; /* 寄存器地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t value = 0; /* 写入值(0x0000-0xFFFF, 大端字节序) */
};

/**
 * @brief 读取异常状态响应, 从站→主站, PDU: 输出数据(1字节, 8个线圈的位状态)
 */
struct ReadExceptionStatusResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint8_t outputData = 0; /* 8个异常输出线圈的当前状态(bit0-bit7对应线圈1-8, 1=ON, 0=OFF) */
};

/**
 * @brief 诊断请求, 主站→从站, PDU: 子功能码(2字节) + 数据(2字节)
 */
struct DiagnosticsRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t subFunc = 0; /* 子功能码(0x0000-0xFFFF, 大端字节序) */
    uint16_t data = 0; /* 子功能数据(0x0000-0xFFFF, 大端字节序) */
};

/**
 * @brief 诊断响应, 从站→主站, PDU: 子功能码(2字节) + 数据(2字节)
 */
struct DiagnosticsResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint16_t subFunc = 0; /* 子功能码(0x0000-0xFFFF, 大端字节序) */
    uint16_t data = 0; /* 子功能数据(0x0000-0xFFFF, 大端字节序) */
};

/**
 * @brief 获取通信事件计数器响应, 从站→主站, PDU: 状态(2字节) + 事件计数(2字节)
 */
struct GetComEventCounterResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint16_t status = 0; /* 设备状态(0xFFFF=忙, 0x0000=就绪, 大端字节序) */
    uint16_t eventCount = 0; /* 事件计数器值(0-65535, 大端字节序) */
};

/**
 * @brief 获取通信事件日志响应, 从站→主站, PDU: 状态(2字节) + 事件计数(2字节) + 消息计数(2字节) + 事件(N字节)
 */
struct GetComEventLogResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint16_t status = 0; /* 设备状态(0xFFFF=忙, 0x0000=就绪, 大端字节序) */
    uint16_t eventCount = 0; /* 事件计数器值(0-65535, 大端字节序) */
    uint16_t messageCount = 0; /* 消息计数器值(0-65535, 大端字节序) */
    const uint8_t* events = nullptr; /* 事件数据(变长, 设备自定义格式) */
    uint8_t eventsLen = 0; /* 事件数据长度(字节数) */
};

/**
 * @brief 报告从站ID响应, 从站→主站, PDU: 从站ID(1字节) + 运行指示(1字节) + 附加数据(N字节)
 */
struct ReportServerIdResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint8_t serverId = 0; /* 从站ID(0x00-0xFF, 设备唯一标识) */
    uint8_t runStatusIndicator = 0; /* 运行状态指示(0x00=停止, 0xFF=运行) */
    const uint8_t* additionalData = nullptr; /* 附加数据(设备特定信息) */
    uint8_t additionalDataLen = 0; /* 附加数据长度(字节数) */
};

/**
 * @brief 写多个线圈请求, 主站→从站, PDU: 起始地址(2) + 数量(2) + 字节计数(1) + 线圈数据(N字节)
 */
struct WriteMultipleCoilsRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t startAddress = 0; /* 起始地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t quantity = 0; /* 写入线圈数量(1-1968, 大端字节序) */
    uint8_t byteCount = 0; /* 后续数据字节数(N = (quantity+7)/8) */
    const uint8_t* data = nullptr; /* 线圈数据(LSB优先, 每个bit代表一个线圈) */
};

/**
 * @brief 写多个线圈响应, 从站→主站, PDU: 起始地址(2字节) + 数量(2字节)
 */
struct WriteMultipleCoilsResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint16_t startAddress = 0; /* 起始地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t quantity = 0; /* 写入线圈数量(1-1968, 大端字节序) */
};

/**
 * @brief 写多个寄存器请求, 主站→从站, PDU: 起始地址(2) + 数量(2) + 字节计数(1) + 寄存器数据(N×2字节)
 */
struct WriteMultipleRegistersRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t startAddress = 0; /* 起始地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t quantity = 0; /* 写入寄存器数量(1-123, 大端字节序) */
    uint8_t byteCount = 0; /* 后续数据字节数(N = quantity×2) */
    const uint8_t* data = nullptr; /* 寄存器数据(每个寄存器2字节, 大端字节序) */
};

/**
 * @brief 写多个寄存器响应, 从站→主站, PDU: 起始地址(2字节) + 数量(2字节)
 */
struct WriteMultipleRegistersResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint16_t startAddress = 0; /* 起始地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t quantity = 0; /* 写入寄存器数量(1-123, 大端字节序) */
};

/**
 * @brief 屏蔽写寄存器请求, 主站→从站, PDU: 地址(2) + AND掩码(2) + OR掩码(2)
 */
struct MaskWriteRegisterRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t address = 0; /* 寄存器地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t andMask = 0; /* AND掩码(0x0000-0xFFFF, 大端字节序) */
    uint16_t orMask = 0; /* OR掩码(0x0000-0xFFFF, 大端字节序) */
};

/**
 * @brief 屏蔽写寄存器响应, 从站→主站, PDU: 地址(2) + AND掩码(2) + OR掩码(2)
 */
struct MaskWriteRegisterResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint16_t address = 0; /* 寄存器地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t andMask = 0; /* AND掩码(0x0000-0xFFFF, 大端字节序) */
    uint16_t orMask = 0; /* OR掩码(0x0000-0xFFFF, 大端字节序) */
};

/**
 * @brief 读写多个寄存器请求, 主站→从站, PDU: 读起始地址(2) + 读数量(2) + 写起始地址(2) + 写数量(2) + 写字节计数(1) + 写数据(N×2)
 */
struct ReadWriteMultipleRegistersRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t readStartAddress = 0; /* 读起始地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t readQuantity = 0; /* 读寄存器数量(1-125, 大端字节序) */
    uint16_t writeStartAddress = 0; /* 写起始地址(0x0000-0xFFFF, 大端字节序) */
    uint16_t writeQuantity = 0; /* 写寄存器数量(1-121, 大端字节序) */
    uint8_t writeByteCount = 0; /* 写数据字节数(N = writeQuantity×2) */
    const uint16_t* writeValues = nullptr; /* 写数据数组(每个寄存器2字节, 大端字节序) */
};

/**
 * @brief 读写多个寄存器响应, 从站→主站, PDU: 字节计数(1字节) + 读数据(N×2字节)
 */
struct ReadWriteMultipleRegistersResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint8_t readByteCount = 0; /* 读数据字节数(N = readQuantity×2) */
    const uint16_t* readValues = nullptr; /* 读数据数组(每个寄存器2字节, 大端字节序) */
};

/**
 * @brief 读FIFO队列请求, 主站→从站, PDU: FIFO指针地址(2字节)
 */
struct ReadFifoQueueRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint16_t fifoPointerAddress = 0; /* FIFO指针地址(0x0000-0xFFFF, 大端字节序) */
};

/**
 * @brief 读FIFO队列响应, 从站→主站, PDU: FIFO计数(2) + FIFO值(N×2) + 后续字节(N×2)
 */
struct ReadFifoQueueResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint16_t fifoCount = 0; /* FIFO中可用的寄存器数量(0-31, 大端字节序) */
    const uint16_t* fifoValues = nullptr; /* FIFO数据值数组(每个值2字节, 大端字节序) */
};

/**
 * @brief 文件记录子请求(读/写共用), 结构: 参考类型(1) + 文件号(2) + 记录号(2) + 记录长度(2)
 */
struct FileRecordSubRequest
{
    uint8_t referenceType = 0; /* 参考类型(通常为0x06, 表示寄存器) */
    uint16_t fileNumber = 0; /* 文件号(0x0000-0xFFFF, 大端字节序) */
    uint16_t recordNumber = 0; /* 记录号(0x0000-0xFFFF, 大端字节序) */
    uint16_t recordLength = 0; /* 记录长度(寄存器数量, 大端字节序) */
};

/**
 * @brief 读文件记录请求, 主站→从站, PDU: 字节计数(1) + 子请求1(7) + ... + 子请求N(7)
 */
struct ReadFileRecordRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint8_t byteCount = 0; /* 后续子请求总字节数(N×7) */
    const uint8_t* subRequests = nullptr; /* 子请求数组(每个7字节) */
};

/**
 * @brief 读文件记录响应, 从站→主站, PDU: 文件记录数据(变长)
 */
struct ReadFileRecordResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    const uint8_t* recordData = nullptr; /* 文件记录数据(每个记录变长, 大端字节序) */
    uint16_t totalDataLen = 0; /* 数据总长度(字节数) */
};

/**
 * @brief 写文件记录请求, 主站→从站, PDU: 字节计数(1) + 子请求1(7+数据) + ... + 子请求N(7+数据)
 */
struct WriteFileRecordRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint8_t byteCount = 0; /* 后续数据总字节数(含子请求和数据) */
    const uint8_t* subRequests = nullptr; /* 子请求数组(每个7字节+数据) */
};

/**
 * @brief 写文件记录响应, 从站→主站, PDU: 请求数据的完整镜像(Echo)
 */
struct WriteFileRecordResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint8_t byteCount = 0; /* 返回相同的请求数据长度 */
    const uint8_t* subRequests = nullptr; /* 返回镜像的请求数据(用于验证) */
};

/**
 * @brief 读取设备标识请求, 主站→从站, PDU: MEI类型(0x0E) + 读设备ID码(1) + 对象ID(1)
 */
struct ReadDeviceIdentificationRequest : public FuncDataSt
{
    bool isRequest() const override
    {
        return true;
    }
    uint8_t meiType = 0x0E; /* MEI类型(固定为0x0E, 标识封装接口) */
    uint8_t readDevIdCode = 0; /* 读设备ID码(0x01=基本,0x02=常规,0x03=扩展,0x04=特定) */
    uint8_t objectId = 0; /* 对象ID(0x00-0xFF, 要读取的对象标识) */
};

/**
 * @brief 读取设备标识响应
 */
struct ReadDeviceIdentificationResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    uint8_t meiType = 0x0E; /* MEI类型(固定0x0E) */
    uint8_t conformityLevel = 0; /* 一致性等级 */
    bool moreFollows = false; /* 后续更多标志 */
    uint8_t nextObjectId = 0; /* 下一个对象ID */
    const uint8_t* objects = nullptr; /* 对象数据(MB编码字符串等) */
    uint8_t objectsLen = 0;
};

/**
 * @brief 通用异常响应(适用于所有功能码)
 */
struct ExceptionResponse : public FuncDataSt
{
    bool isRequest() const override
    {
        return false;
    }
    ExceptionCode code = ExceptionCode::ILLEGAL_FUNCTION; /* 异常码 */
};

/**
 * @brief Modbus数据
 */
struct DataSt
{
    uint16_t transactionId = 0; /* 事务ID(TCP)或自增ID(RTU) */
    uint8_t slaveAddress = 0; /* 从站地址 */
    bool isBroadcast = false; /* 是否为广播地址(RTU地址0) */
    FunctionCode funcCode; /* 功能码 */
    bool isRequest = false; /* 是否请求包, true-主站请求包, false-从站响应包 */
    bool isException = false; /* 是否为异常响应 */
    uint32_t bizDataLen = 0; /* 业务数据长度 */
    const uint8_t* rawData = nullptr; /* 原始数据 */
    uint32_t rawDataLen = 0; /* 原始数据长度 */
    uint32_t funcDataOffset = 0; /* 功能数据偏移(基于rawData的偏移), 0表示没有功能数据 */
    uint32_t funcDataLen = 0; /* 功能数据长度, 0表示没有功能数据 */
};

/**
 * @brief 获取最小帧数据长度(不含地址, 功能码, CRC)
 * @param code 功能码
 * @param isException 是否为异常响应模式
 * @return 最小帧数据长度
 */
inline uint32_t getMinFrameLength(FunctionCode code, bool isException)
{
    if (isException) /* 异常响应模式: 固定返回1字节(异常码) */
    {
        return 1;
    }
    switch (code)
    {
    /* 基础读写功能码 */
    case FunctionCode::READ_COILS:
    case FunctionCode::READ_DISCRETE_INPUTS:
    case FunctionCode::READ_HOLDING_REGISTERS:
    case FunctionCode::READ_INPUT_REGISTERS:
        return 4; /* 起始地址(2) + 数量(2) */
    case FunctionCode::WRITE_SINGLE_COIL:
    case FunctionCode::WRITE_SINGLE_REGISTER:
        return 4; /* 地址(2) + 值(2) */
    /* 纯功能码 */
    case FunctionCode::READ_EXCEPTION_STATUS:
    case FunctionCode::GET_COM_EVENT_COUNTER:
    case FunctionCode::GET_COM_EVENT_LOG:
    case FunctionCode::REPORT_SERVER_ID:
        return 0; /* 仅功能码, 无数据 */
    /* 诊断功能码 */
    case FunctionCode::DIAGNOSTICS:
        return 2; /* 子功能码(2) */
    /* 批量操作功能码 */
    case FunctionCode::WRITE_MULTIPLE_COILS:
    case FunctionCode::WRITE_MULTIPLE_REGISTERS:
        return 5; /* 起始地址(2) + 数量(2) + 字节计数(1) */
    case FunctionCode::MASK_WRITE_REGISTER:
        return 6; /* 地址(2) + AND掩码(2) + OR掩码(2) */
    case FunctionCode::READ_WRITE_MULTIPLE_REGISTERS:
        return 9; /* 读起始地址(2) + 读数量(2) + 写起始地址(2) + 写数量(2) + 写字节计数(1) */
    case FunctionCode::READ_FIFO_QUEUE:
        return 2; /* 队列指针地址(2) */
    case FunctionCode::READ_FILE_RECORD:
    case FunctionCode::WRITE_FILE_RECORD:
        return 8; /* 字节计数(1) + 最少1个子请求(7) */
    case FunctionCode::READ_DEVICE_IDENTIFICATION:
        return 3; /* MEI类型(1) + 读设备ID码(1) + 对象ID(1) */
    default:
        return 0; /* 默认无数据 */
    }
}

/**
 * @brief 验证功能码有效性
 * @param funcCode 功能码
 * @return true-有效, false-无效
 */
inline bool isValidFunctionCode(uint8_t funcCode)
{
    funcCode &= 0x7F; /* 清除异常标记 */
    switch (funcCode)
    {
    case FunctionCode::READ_COILS:
    case FunctionCode::READ_DISCRETE_INPUTS:
    case FunctionCode::READ_HOLDING_REGISTERS:
    case FunctionCode::READ_INPUT_REGISTERS:
    case FunctionCode::WRITE_SINGLE_COIL:
    case FunctionCode::WRITE_SINGLE_REGISTER:
    case FunctionCode::READ_EXCEPTION_STATUS:
    case FunctionCode::DIAGNOSTICS:
    case FunctionCode::GET_COM_EVENT_COUNTER:
    case FunctionCode::GET_COM_EVENT_LOG:
    case FunctionCode::WRITE_MULTIPLE_COILS:
    case FunctionCode::WRITE_MULTIPLE_REGISTERS:
    case FunctionCode::REPORT_SERVER_ID:
    case FunctionCode::READ_FILE_RECORD:
    case FunctionCode::WRITE_FILE_RECORD:
    case FunctionCode::MASK_WRITE_REGISTER:
    case FunctionCode::READ_WRITE_MULTIPLE_REGISTERS:
    case FunctionCode::READ_FIFO_QUEUE:
    case FunctionCode::READ_DEVICE_IDENTIFICATION:
        return true;
    default:
        return false;
    }
}

/**
 * @brief 解析功能数据(转为可直接阅读的有意义的数据类型)
 * @param funcCode 功能码
 * @param isRequest 是否请求数据包
 * @param isException 是否异常
 * @param funcData 功能数据
 * @param funcDataLen 功能数据长度
 * @return 功能数据
 */
static std::shared_ptr<FuncDataSt> parseFuncData(FunctionCode funcCode, bool isRequest, bool isException, const uint8_t* funcData,
                                                 uint32_t funcDataLen)
{
    if (!funcData || 0 == funcDataLen)
    {
        return nullptr;
    }
    if (isException)
    {
        auto resp = std::make_shared<modbus::ExceptionResponse>();
        resp->code = (modbus::ExceptionCode)(funcData[0]);
        return resp;
    }
    switch (funcCode)
    {
    case FunctionCode::READ_COILS:
        if (isRequest && 4 == funcDataLen) /* 请求 */
        {
            auto req = std::make_shared<ReadCoilsRequest>();
            req->startAddress = (funcData[0] << 8) | funcData[1];
            req->quantity = (funcData[2] << 8) | funcData[3];
            return req;
        }
        else if (!isRequest && funcDataLen >= 1) /* 响应 */
        {
            auto resp = std::make_shared<ReadCoilsResponse>();
            resp->byteCount = funcData[0];
            resp->status = (funcDataLen > 1) ? (funcData + 1) : nullptr;
            return resp;
        }
        break;
    case FunctionCode::READ_DISCRETE_INPUTS:
        if (isRequest && 4 == funcDataLen) /* 请求 */
        {
            auto req = std::make_shared<ReadDiscreteInputsRequest>();
            req->startAddress = (funcData[0] << 8) | funcData[1];
            req->quantity = (funcData[2] << 8) | funcData[3];
            return req;
        }
        else if (!isRequest && funcDataLen >= 1) /* 响应 */
        {
            auto resp = std::make_shared<ReadDiscreteInputsResponse>();
            resp->byteCount = funcData[0];
            resp->status = (funcDataLen > 1) ? (funcData + 1) : nullptr;
            return resp;
        }
        break;
    case FunctionCode::READ_HOLDING_REGISTERS:
        if (isRequest && 4 == funcDataLen) /* 请求 */
        {
            auto req = std::make_shared<ReadHoldingRegistersRequest>();
            req->startAddress = (funcData[0] << 8) | funcData[1];
            req->quantity = (funcData[2] << 8) | funcData[3];
            return req;
        }
        else if (!isRequest && funcDataLen >= 1) /* 响应 */
        {
            auto resp = std::make_shared<ReadHoldingRegistersResponse>();
            resp->byteCount = funcData[0];
            resp->values = (const uint16_t*)(funcData + 1);
            return resp;
        }
        break;
    case FunctionCode::READ_INPUT_REGISTERS:
        if (isRequest && 4 == funcDataLen) /* 请求 */
        {
            auto req = std::make_shared<ReadInputRegistersRequest>();
            req->startAddress = (funcData[0] << 8) | funcData[1];
            req->quantity = (funcData[2] << 8) | funcData[3];
            return req;
        }
        else if (!isRequest && funcDataLen >= 1) /* 响应 */
        {
            auto resp = std::make_shared<ReadInputRegistersResponse>();
            resp->byteCount = funcData[0];
            resp->values = (const uint16_t*)(funcData + 1);
            return resp;
        }
        break;
    case FunctionCode::WRITE_SINGLE_COIL:
        if (funcDataLen >= 4)
        {
            uint16_t addr = (funcData[0] << 8) | funcData[1];
            uint16_t val = (funcData[2] << 8) | funcData[3];
            bool isOn = (0xFF00 == val);
            if (isRequest) /* 请求 */
            {
                auto req = std::make_shared<WriteSingleCoilRequest>();
                req->address = addr;
                req->value = isOn;
                return req;
            }
            else /* 响应 */
            {
                auto resp = std::make_shared<WriteSingleCoilResponse>();
                resp->address = addr;
                resp->value = isOn;
                return resp;
            }
        }
        break;
    case FunctionCode::WRITE_SINGLE_REGISTER:
        if (funcDataLen >= 4)
        {
            uint16_t addr = (funcData[0] << 8) | funcData[1];
            uint16_t val = (funcData[2] << 8) | funcData[3];
            if (isRequest) /* 请求 */
            {
                auto req = std::make_shared<WriteSingleRegisterRequest>();
                req->address = addr;
                req->value = val;
                return req;
            }
            else /* 响应 */
            {
                auto resp = std::make_shared<WriteSingleRegisterResponse>();
                resp->address = addr;
                resp->value = val;
                return resp;
            }
        }
        break;
    case FunctionCode::READ_EXCEPTION_STATUS:
        if (!isRequest && 1 == funcDataLen) /* 响应 */
        {
            auto resp = std::make_shared<ReadExceptionStatusResponse>();
            resp->outputData = funcData[0];
            return resp;
        }
        break;
    case FunctionCode::DIAGNOSTICS:
        if (funcDataLen >= 4)
        {
            uint16_t subFunc = (funcData[0] << 8) | funcData[1];
            uint16_t dataTmp = (funcData[2] << 8) | funcData[3];
            if (isRequest) /* 请求 */
            {
                auto req = std::make_shared<DiagnosticsRequest>();
                req->subFunc = subFunc;
                req->data = dataTmp;
                return req;
            }
            else /* 响应 */
            {
                auto resp = std::make_shared<DiagnosticsResponse>();
                resp->subFunc = subFunc;
                resp->data = dataTmp;
                return resp;
            }
        }
        break;
    case FunctionCode::GET_COM_EVENT_COUNTER:
        if (!isRequest && 4 == funcDataLen) /* 响应 */
        {
            auto resp = std::make_shared<GetComEventCounterResponse>();
            resp->status = (funcData[0] << 8) | funcData[1];
            resp->eventCount = (funcData[2] << 8) | funcData[3];
            return resp;
        }
        break;
    case FunctionCode::GET_COM_EVENT_LOG:
        if (!isRequest && funcDataLen >= 6) /* 响应 */
        {
            auto resp = std::make_shared<GetComEventLogResponse>();
            resp->status = (funcData[0] << 8) | funcData[1];
            resp->eventCount = (funcData[2] << 8) | funcData[3];
            resp->messageCount = (funcData[4] << 8) | funcData[5];
            resp->events = (funcDataLen > 6) ? (funcData + 6) : nullptr;
            resp->eventsLen = (funcDataLen > 6) ? (funcDataLen - 6) : 0;
            return resp;
        }
        break;
    case FunctionCode::REPORT_SERVER_ID:
        if (!isRequest && funcDataLen >= 2) /* 响应 */
        {
            auto resp = std::make_shared<ReportServerIdResponse>();
            resp->serverId = funcData[0];
            resp->runStatusIndicator = funcData[1];
            resp->additionalData = (funcDataLen > 2) ? (funcData + 2) : nullptr;
            resp->additionalDataLen = (funcDataLen > 2) ? (funcDataLen - 2) : 0;
            return resp;
        }
        break;
    case FunctionCode::WRITE_MULTIPLE_COILS:
        if (isRequest && funcDataLen >= 5) /* 请求 */
        {
            auto req = std::make_shared<WriteMultipleCoilsRequest>();
            req->startAddress = (funcData[0] << 8) | funcData[1];
            req->quantity = (funcData[2] << 8) | funcData[3];
            req->byteCount = funcData[4];
            req->data = funcData + 5;
            return req;
        }
        else if (!isRequest && 4 == funcDataLen) /* 响应 */
        {
            auto resp = std::make_shared<WriteMultipleCoilsResponse>();
            resp->startAddress = (funcData[0] << 8) | funcData[1];
            resp->quantity = (funcData[2] << 8) | funcData[3];
            return resp;
        }
        break;
    case FunctionCode::WRITE_MULTIPLE_REGISTERS:
        if (isRequest && funcDataLen >= 5) /* 请求 */
        {
            auto req = std::make_shared<WriteMultipleRegistersRequest>();
            req->startAddress = (funcData[0] << 8) | funcData[1];
            req->quantity = (funcData[2] << 8) | funcData[3];
            req->byteCount = funcData[4];
            req->data = funcData + 5;
            return req;
        }
        else if (!isRequest && 4 == funcDataLen) /* 响应 */
        {
            auto resp = std::make_shared<WriteMultipleRegistersResponse>();
            resp->startAddress = (funcData[0] << 8) | funcData[1];
            resp->quantity = (funcData[2] << 8) | funcData[3];
            return resp;
        }
        break;
    case FunctionCode::MASK_WRITE_REGISTER:
        if (funcDataLen >= 6)
        {
            uint16_t addr = (funcData[0] << 8) | funcData[1];
            uint16_t andMask = (funcData[2] << 8) | funcData[3];
            uint16_t orMask = (funcData[4] << 8) | funcData[5];
            if (isRequest) /* 请求 */
            {
                auto req = std::make_shared<MaskWriteRegisterRequest>();
                req->address = addr;
                req->andMask = andMask;
                req->orMask = orMask;
                return req;
            }
            else /* 响应 */
            {
                auto resp = std::make_shared<MaskWriteRegisterResponse>();
                resp->address = addr;
                resp->andMask = andMask;
                resp->orMask = orMask;
                return resp;
            }
        }
        break;
    case FunctionCode::READ_WRITE_MULTIPLE_REGISTERS:
        if (isRequest && funcDataLen >= 9) /* 请求 */
        {
            auto req = std::make_shared<ReadWriteMultipleRegistersRequest>();
            req->readStartAddress = (funcData[0] << 8) | funcData[1];
            req->readQuantity = (funcData[2] << 8) | funcData[3];
            req->writeStartAddress = (funcData[4] << 8) | funcData[5];
            req->writeQuantity = (funcData[6] << 8) | funcData[7];
            req->writeByteCount = funcData[8];
            req->writeValues = (const uint16_t*)(funcData + 9);
            return req;
        }
        else if (!isRequest && funcDataLen >= 1) /* 响应 */
        {
            auto resp = std::make_shared<ReadWriteMultipleRegistersResponse>();
            resp->readByteCount = funcData[0];
            resp->readValues = (const uint16_t*)(funcData + 1);
            return resp;
        }
        break;
    case FunctionCode::READ_FIFO_QUEUE:
        if (isRequest && 2 == funcDataLen) /* 请求 */
        {
            auto req = std::make_shared<ReadFifoQueueRequest>();
            req->fifoPointerAddress = (funcData[0] << 8) | funcData[1];
            return req;
        }
        else if (!isRequest && funcDataLen >= 2) /* 响应 */
        {
            auto resp = std::make_shared<ReadFifoQueueResponse>();
            resp->fifoCount = (funcData[0] << 8) | funcData[1];
            resp->fifoValues = (const uint16_t*)(funcData + 2);
            return resp;
        }
        break;
    case FunctionCode::READ_FILE_RECORD:
        if (isRequest && funcDataLen >= 1) /* 请求 */
        {
            auto req = std::make_shared<ReadFileRecordRequest>();
            req->byteCount = funcData[0];
            req->subRequests = funcData + 1;
            return req;
        }
        else if (!isRequest && funcDataLen >= 2) /* 响应 */
        {
            auto resp = std::make_shared<ReadFileRecordResponse>();
            resp->recordData = funcData;
            resp->totalDataLen = funcDataLen;
            return resp;
        }
        break;
    case FunctionCode::WRITE_FILE_RECORD:
        if (isRequest && funcDataLen >= 7) /* 请求 */
        {
            auto req = std::make_shared<WriteFileRecordRequest>();
            req->byteCount = funcData[0];
            req->subRequests = funcData + 1;
            return req;
        }
        else if (!isRequest && funcDataLen >= 1) /* 响应, 响应数据是请求数据的镜像(Echo) */
        {
            auto resp = std::make_shared<WriteFileRecordResponse>();
            resp->byteCount = funcData[0];
            resp->subRequests = funcData + 1;
            return resp;
        }
        break;
    case FunctionCode::READ_DEVICE_IDENTIFICATION:
        if (isRequest && 3 == funcDataLen) /* 请求 */
        {
            auto req = std::make_shared<ReadDeviceIdentificationRequest>();
            req->meiType = funcData[0];
            req->readDevIdCode = funcData[1];
            req->objectId = funcData[2];
            return req;
        }
        else if (!isRequest && funcDataLen >= 3) /* 响应 */
        {
            auto resp = std::make_shared<ReadDeviceIdentificationResponse>();
            resp->meiType = funcData[0];
            resp->conformityLevel = funcData[1] & 0x7F;
            resp->moreFollows = 0 != (funcData[1] & 0x80);
            resp->nextObjectId = funcData[2];
            resp->objects = (funcDataLen > 3) ? (funcData + 3) : nullptr;
            resp->objectsLen = (funcDataLen > 3) ? (funcDataLen - 3) : 0;
            return resp;
        }
        break;
    default:
        break;
    }
    return nullptr;
}

/**
 * @brief 解析功能数据(转为可直接阅读的有意义的数据类型)
 * @param data Modbus数据
 * @return 功能数据
 */
static std::shared_ptr<FuncDataSt> parseFuncData(const DataSt& data)
{
    if (!data.rawData || 0 == data.rawDataLen || 0 == data.funcDataOffset || 0 == data.funcDataLen
        || (data.funcDataOffset + data.funcDataLen) >= data.rawDataLen)
    {
        return nullptr;
    }
    return parseFuncData(data.funcCode, data.isRequest, data.isException, data.rawData + data.funcDataOffset, data.funcDataLen);
}
} // namespace modbus
} // namespace npacket
