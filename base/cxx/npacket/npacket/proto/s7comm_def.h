#pragma once
#include <memory>
#include <vector>

#include "tpkt_cotp.h"

namespace npacket
{
/**
 * @brief S7COMM 协议相关数据定义
 */
namespace s7comm
{
/**
 * @brief S7COMM消息类型(ROSCTR)
 */
enum class RosctrType
{
    JOB = 0x01, /* 作业请求: 主动发起的操作请求(如读/写变量) */
    ACK = 0x02, /* 确认响应: 仅确认请求接收, 无数据返回 */
    ACK_DATA = 0x03, /* 确认数据响应: 确认+返回请求数据(如读变量结果) */
    JOB_ACK = 0x04, /* 作业+确认: 复合类型, 请求+确认一体(少见) */
    DATA = 0x06, /* 纯数据传输: 无确认的单向数据推送 */
    USERDATA = 0x07, /* 原始协议扩展: 自定义/扩展协议数据 */
    NEGOTIATE = 0x08, /* 协商请求: 协商通信参数(如PDU大小) */
    CONFIRM = 0x09, /* 协商确认: 响应协商请求, 确认参数 */
    ABORT = 0x0A /* 连接中止: 异常终止连接/作业(如超时, 错误) */
};

/**
 * @brief S7COMM功能码(参数第一个字节)
 */
enum class FunctionCode
{
    CPU_SERVICES = 0x00, /* CPU底层服务 */
    READ_VARIABLE = 0x04, /* 读变量(标准) */
    WRITE_VARIABLE = 0x05, /* 写变量(标准) */
    /* 块下载/上传 */
    REQUEST_DOWNLOAD = 0x1A, /* 请求下载块 */
    DOWNLOAD_BLOCK = 0x1B, /* 下载块数据 */
    DOWNLOAD_ENDED = 0x1C, /* 下载结束 */
    START_UPLOAD = 0x1D, /* 开始上传 */
    UPLOAD = 0x1E, /* 上传块数据 */
    END_UPLOAD = 0x1F, /* 上传结束 */
    /* PLC控制 */
    PLC_CONTROL = 0x26, /* PLC控制(启动/冷启动/暖启动) */
    PI_SERVICE = 0x28, /* PI服务(外设接口服务) */
    PLC_STOP = 0x29, /* PLC停止 */
    /* 存储/块操作 */
    COPY_RAM_TO_ROM = 0x2A, /* RAM拷贝到ROM */
    COMPRESS = 0x2B, /* 压缩内存 */
    DELETE_BLOCK = 0x2C, /* 删除块 */
    REPLACE_BLOCK = 0x2D, /* 替换块 */
    BLOCK_STATUS = 0x2E, /* 查询块状态 */
    /* 通信协商 */
    SETUP_COMMUNICATION = 0xF0, /* 建立通信(PDU协商) */
};

/**
 * @brief S7COMM头部信息(10或12字节)
 */
struct S7Header
{
    uint8_t protocolId = 0; /* 协议ID, 固定为0x32 */
    RosctrType rosctr; /* 消息类型 */
    uint16_t redundancyIdentification = 0; /* 冗余数据, 通常为0x0000 */
    uint16_t protocolDataUnitReference = 0; /* PDU参考, 请求/响应配对用 */
    uint16_t parameterLength = 0; /* 参数长度 */
    uint16_t dataLength = 0; /* 数据长度 */
    uint8_t errorClass = 0; /* 错误类型(仅当rosctr的值为ACK_DATA时候有效) */
    uint8_t errorCode = 0; /* 错误代码(仅当rosctr的值为ACK_DATA时候有效) */
};

/**
 * @brief 变量寻址规范
 */
struct ItemSpec
{
    uint8_t variableSpecification = 0; /* 变量规范类型, 通常为0x12 */
    uint8_t addressLength = 0; /* 地址长度 */
    uint8_t syntaxId = 0; /* 语法ID, 0x10=S7ANY, 0x82=DBREAD */
    uint8_t transportSize = 0; /* 传输大小 */
    uint16_t length = 0; /* 数据长度(元素个数) */
    uint16_t dbNumber = 0; /* DB块号 */
    uint8_t area = 0; /* 存储区 */
    uint8_t address[3] = {0}; /* 字节地址(3字节) */
};

/**
 * @brief 读/写参数
 */
struct ReadWriteParam
{
    uint8_t functionCode = 0; /* 功能码 */
    uint8_t itemCount = 0; /* 条目数 */
    std::vector<ItemSpec> items; /* 变量条目列表 */
};

/**
 * @brief 请求下载参数(0x1A)
 */
struct RequestDownloadParam
{
    uint8_t functionCode = 0;
    uint8_t unknown1 = 0; /* 通常为0x01 */
    uint8_t unknown2 = 0; /* 通常为0x00 */
    uint8_t blockTypeLen = 0; /* 块类型长度, 通常为2 */
    uint8_t blockNumLen = 0; /* 块号长度, 通常为5 */
    uint8_t fileSystemLen = 0; /* 文件系统长度, 通常为1 */
    uint16_t blockType = 0; /* ASCII编码, 如'0B'=DB */
    char blockNumber[6] = {0}; /* ASCII编码块号 */
    char fileSystem = 0; /* 'A'=Active, 'P'=Passive */
};

/**
 * @brief 开始上传参数(0x1D)
 */
struct StartUploadParam
{
    uint8_t functionCode = 0;
    uint8_t blockTypeLen = 0;
    uint8_t blockNumLen = 0;
    uint8_t fileSystemLen = 0;
    uint16_t blockType = 0;
    char blockNumber[6] = {0};
    char fileSystem = 0;
};

/**
 * @brief 上传块数据参数(0x1E)
 */
struct UploadParam
{
    uint8_t functionCode = 0;
    uint32_t uploadId = 0; /* 上传会话ID(大端) */
};

/**
 * @brief 上传结束参数(0x1F)
 */
struct EndUploadParam
{
    uint8_t functionCode = 0;
    uint32_t uploadId = 0; /* 上传会话ID (大端) */
};

/**
 * @brief PLC控制参数(0x26)
 */
struct PlcControlParam
{
    uint8_t functionCode = 0;
    uint8_t paramCount = 0; /* 参数字符串数量 */
    std::vector<std::string> params; /* 参数列表, 如"P_PROGRAM" */
};

/**
 * @brief PI服务参数(0x28)
 */
struct PiServiceParam
{
    uint8_t functionCode = 0;
    uint8_t paramCount = 0;
    std::vector<std::string> params; /* PI服务名及参数, 如"_MODU" */
};

/**
 * @brief PLC停止参数(0x29)
 */
struct PlcStopParam
{
    uint8_t functionCode = 0;
};

/**
 * @brief 块操作参数(0x2C/0x2D/0x2E等)
 */
struct BlockOperationParam
{
    uint8_t functionCode = 0;
    uint8_t paramCount = 0;
    uint16_t blockType = 0; /* ASCII编码 */
    char blockNumber[6] = {0}; /* ASCII编码块号 */
    char fileSystem = 0; /* 'A' or 'P' */
};

/**
 * @brief 通信协商参数(0xF0)
 */
struct SetupCommParam
{
    uint8_t functionCode = 0;
    uint8_t reserved = 0;
    uint16_t maxAmqCaller = 0; /* 请求方最大并行作业数 */
    uint16_t maxAmqCallee = 0; /* 响应方最大并行作业数 */
    uint16_t pduLength = 0; /* 协商PDU长度 */
};

/**
 * @brief 数据项
 */
struct DataItem
{
    uint8_t returnCode = 0; /* 返回码 */
    uint8_t transportSize = 0; /* 传输大小 */
    uint16_t length = 0; /* 数据长度(位为单位时*8) */
    const uint8_t* data = nullptr; /* 实际数据 */
};

/**
 * @brief S7COMM完整信息
 */
struct S7CommInfo
{
    S7Header header; /* 头部 */

    const uint8_t* rawParameter = nullptr; /* 原始参数域 */
    const uint8_t* rawData = nullptr; /* 原始数据域 */

    uint8_t functionCode = 0; /* 当前功能码 */
    ReadWriteParam rwParam; /* 读/写参数(0x04/0x05) */
    RequestDownloadParam reqDlParam; /* 请求下载(0x1A) */
    StartUploadParam startUlParam; /* 开始上传(0x1D) */
    UploadParam uploadParam; /* 上传(0x1E) */
    EndUploadParam endUlParam; /* 结束上传(0x1F) */
    PlcControlParam plcCtrlParam; /* PLC控制(0x26) */
    PiServiceParam piParam; /* PI服务(0x28) */
    PlcStopParam plcStopParam; /* PLC停止(0x29) */
    BlockOperationParam blockOpParam; /* 块操作(0x2C-0x2E) */
    SetupCommParam setupParam; /* 通信协商(0xF0) */
    std::vector<DataItem> dataItems; /* 数据项列表 */
};
} // namespace s7comm
} // namespace npacket
