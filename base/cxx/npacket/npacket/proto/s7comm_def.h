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
    UPLOAD_BLOCK = 0x1E, /* 上传块数据 */
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
 * @brief CPU服务参数项(用于USERDATA类型的参数寻址)
 */
struct CpuServiceParamItem
{
    uint8_t variableSpecification = 0; /* 变量规范: 0x12 */
    uint8_t lengthOfFollowingAddressSpecification = 0; /* 后续地址规范长度: 4或8 */
    uint8_t syntaxId = 0; /* 语法ID: 0x11=ParameterShort, 0x12=ParameterExtended */
    uint8_t type = 0; /* 类型(高2位): 1-Request, 2-Response */
    /* 功能组(低6位): 0x00-Mode transition(模式切换), 0x01-Programmer commands(编程器命令), 0x02-Cyclic data(循环数据), 0x03-Block functions(块功能), 
                    0x04-CPU functions(CPU功能), 0x05-Security(安全功能), 0x06-PBC(过程块通信), 0x07-Time functions(时间功能) */
    uint8_t functionGroup = 0;
    /* 子功能
        - 当functionGroup的值为0x00时, 取值如下:
        0x00-STOP, 停止PLC
        0x01-Warm restart, 暖启动(保留数据块内容)
        0x02-RUN, 运行模式
        0x03-Hot restart, 热启动(S7-400特有, 尽可能在中断点恢复)
        0x04-Cold restart, 冷启动(清除所有数据, 重新初始化)
        - 当functionGroup的值为0x03时, 取值如下:
        0x01-List blocks, 列出所有程序块
        0x02-List blocks of type, 按类型列出程序块(如所有DB块)
        0x03-Get block info, 获取指定块的信息(大小, 时间戳等)
        - 当functionGroup的值为0x04时, 取值如下:
        0x01-Read SZL, 读取系统状态列表: 读取CPU诊断信息、模块状态、通信状态等
        0x02-Message service, 消息服务: 异步消息通知(如报警)
        0x03-Diagnostic message, 诊断消息: 读取诊断缓冲区条目
        0x04-ALARM, 报警服务: 处理PLC报警(ALARM_SQ, ALARM_S等)
        0x05-ALARM_8, 8通道报警: 用于S7-300/400的ALARM_8块
        0x06-ALARM_8P, 8通道报警带参数: 带过程值的 ALARM_8
        0x07-NOTIFY, 通知服务: 无确认的通知消息
        0x08-AR_SEND, 归档发送: 发送归档数据到HMI/SCADA
        0x09-0x0F: 保留
        0x10-Time functions, 时间功能: 读取/设置CPU时钟
        0x11-Time functions (Set), 设置时间: 写CPU系统时间
        0x12-Read clock, 读取时钟: 读取当前CPU时间
        0x13-Set clock, 设置时钟: 同步CPU时间
        0x14-Communication status, 通信状态: 读取通信连接状态
        0x15-Block status, 块状态: 查询程序块运行状态
        0x16-CPU status, CPU状态: 读取CPU运行模式(RUN/STOP/STARTUP)
        - 当functionGroup的值为0x07时, 取值如下:
        0x01-Read clock, 读取时钟: 读取当前CPU时间
        0x02-Set clock, 设置时钟: 同步CPU时间
     */
    uint8_t subFunction = 0;
    uint8_t sequenceNumber = 0; /* 序列号 */
    uint8_t dataUnitReferenceNumber = 0; /* 数据单元参考号(syntaxId值为0x12时的特有字段) */
    uint8_t lastDataUnit = 0; /* 最后数据单元(syntaxId值为0x12时的特有字段): 0=Yes */
    uint16_t errorCode = 0; /* 错误码(syntaxId值为0x12时的特有字段) */
};

/**
 * @brief CPU服务参数(0x00)
 */
struct CpuServiceParam
{
    uint8_t itemCount = 0; /* 条目数 */
    std::vector<CpuServiceParamItem> items; /* 参数项列表 */
};

/**
 * @brief 块列表条目
 */
struct BlockListItem
{
    uint8_t blockType = 0; /* 块类型 */
    uint16_t blockCount = 0; /* 该类型块的数量 */
};

/**
 * @brief 按类型列出程序块(请求)
 */
struct BlockListOfTypeRequest
{
    uint8_t blockType = 0; /* 块类型 */
};

/**
 * @brief 按类型列出程序块列表条目
 */
struct BlockListOfTypeItem
{
    uint16_t blockNumber = 0; /* 块编号 */
    uint8_t blockFlags = 0; /* 块标识 */
    uint8_t blockLanguage = 0; /* 块语言 */
};

/**
 * @brief 按类型列出程序块(响应)
 */
struct BlockListOfTypeResponse
{
    std::vector<BlockListOfTypeItem> items; /* 块列表 */
};

/**
 * @brief 按类型列出程序块数据
 */
struct BlockListOfTypeData
{
    bool isRequest = false; /* true=请求, false=响应 */
    BlockListOfTypeRequest req; /* 请求数据 */
    BlockListOfTypeResponse resp; /* 响应数据 */
};

/**
 * @brief 块信息数据(请求)
 */
struct BlockInfoRequest
{
    uint8_t blockType = 0; /* 块类型 */
    uint32_t blockNumber = 0; /* 块号 */
    uint8_t fileSystem = 0; /* 文件系统(ASCII): 'A'=Active(0x41), 'P'=Passive(0x50), 'B'=Both(0x42) */
};

/**
 * @brief 块信息数据(响应)
 */
struct BlockInfoResponse
{
    uint16_t blockType = 0; /* 块类型(大端) */
    uint16_t lengthOfInfo = 0; /* 信息长度 */
    uint16_t unknownBlockinfo2 = 0; /* 未知 */
    uint16_t constant3 = 0; /* 常量 "pp" = 0x7070 */
    uint8_t unknownByte1 = 0; /* 未知字节 */
    uint8_t rawBlockFlags = 0; /* 原始块标志 */
    uint8_t blockFlagsLinked = 0; /* 块标志: bit0, 是否链接 */
    uint8_t blockFlagsStandardBlock = 0; /* 块标志: bit1, 是否为标准块 */
    uint8_t blockFlagsNonRetain = 0; /* 块标志: bit3, 是否非保持 */
    uint8_t blockLanguage = 0; /* 块语言 */
    uint8_t subblkType = 0; /* 子块类型 */
    uint16_t blockNumber = 0; /* 块号(大端) */
    uint32_t lengthLoadMemory = 0; /* 装载内存长度(大端) */
    uint32_t blockSecurity = 0; /* 块安全等级: 4字节, 0=None */
    uint8_t codeTimestamp[6] = {0}; /* 代码时间戳: 6字节 */
    uint8_t interfaceTimestamp[6] = {0}; /* 接口时间戳: 6字节 */
    uint16_t ssbLength = 0; /* SSB长度 */
    uint16_t addLength = 0; /* ADD长度 */
    uint16_t localDataLength = 0; /* 本地数据长度 */
    uint16_t mc7CodeLength = 0; /* MC7代码长度 */
    char author[9] = {0}; /* 作者(字符串) */
    char family[9] = {0}; /* 家族(字符串) */
    char name[9] = {0}; /* 名称(字符串) */
    uint8_t versionMajor = 0; /* 版本主号(高4位) */
    uint8_t versionMinor = 0; /* 版本次号(低4位) */
    uint8_t unknownByte2 = 0; /* 未知字节 */
    uint16_t blockChecksum = 0; /* 块校验和(大端) */
    uint32_t reserved1 = 0; /* 保留1 */
    uint32_t reserved2 = 0; /* 保留2 */
};

/**
 * @brief 块信息数据
 */
struct BlockInfoData
{
    bool isRequest = false; /* true=请求, false=响应 */
    BlockInfoRequest req; /* 请求数据 */
    BlockInfoResponse resp; /* 响应数据 */
};

/**
 * @brief SZL-ID
 */
struct SzlId
{
    uint16_t rawId = 0; /* 原始SZL-ID */
    uint8_t diagnosticType = 0; /* 诊断类型: 高4位 (0x0=CPU, 0x1=模块等) */
    uint8_t extractNumber = 0; /* 提取号: 中间4位 */
    uint8_t partialListId = 0; /* 列表号: 低8位 */
};

/**
 * @brief SZL头部(8字节)
 */
struct SzlHeader
{
    SzlId szlId; /* SZL-ID */
    uint16_t szlIndex = 0; /* SZL-Index */
    uint16_t listLength = 0; /* 每个记录的长度(bytes) */
    uint16_t listCount = 0; /* 记录数量 */
};

/**
 * @brief SZL数据(变长, 由listLength决定)
 */
struct SzlData
{
    uint16_t length = 0; /* 数据长度 */
    const uint8_t* data = nullptr; /* 原始数据域 */
};

/**
 * @brief 消息服务数据(请求)
 */
struct MessageServiceRequest
{
    uint8_t modeTransition = 0; /* 订阅事件: 模式切换 */
    uint8_t systemDiagnostics = 0; /* 订阅事件: 系统诊断 */
    uint8_t userDefined = 0; /* 订阅事件: 用户自定义 */
    uint8_t alarms = 0; /* 订阅事件: 报警 */
    uint8_t reserved = 0; /* 保留/未知字节 */
    std::string username; /* 用户名(ASCII, 变长) */
};

/**
 * @brief 消息服务数据(响应)
 */
struct MessageServiceResponse
{
    uint8_t result = 0; /* 结果码: 0x02=成功订阅等 */
    uint8_t reserved = 0; /* 保留/未知字节 */
};

/**
 * @brief 消息服务数据(Message service)
 */
struct MessageServiceData
{
    bool isRequest = false; /* true=请求, false=响应 */
    MessageServiceRequest req; /* 请求数据 */
    MessageServiceResponse resp; /* 响应数据 */
};

/**
 * @brief 时间戳结构(10字节, BCD编码), 用于CPU Services的Read/Set clock功能
 */
struct S7Timestamp
{
    uint8_t reserved = 0; /* 保留字节 */
    uint8_t year1 = 0; /* 年份十位(BCD) */
    uint8_t year2 = 0; /* 年份个位(BCD) */
    uint8_t month = 0; /* 月份(BCD) */
    uint8_t day = 0; /* 日期(BCD) */
    uint8_t hour = 0; /* 小时(BCD) */
    uint8_t minute = 0; /* 分钟(BCD) */
    uint8_t second = 0; /* 秒(BCD) */
    uint16_t milliseconds = 0; /* 毫秒(12位) */
    uint8_t weekday = 0; /* 星期: 1=Sun, ..., 7=Sat */
};

/**
 * @brief CPU服务数据
 */
struct CpuServiceData
{
    uint8_t returnCode = 0; /* 返回码: 0xff=Success, 0x0a=Object does not exist等 */
    uint8_t transportSize = 0; /* 传输大小: 0x09=OCTET STRING, 0x00=NULL */
    uint16_t length = 0; /* 数据长度 */
    const uint8_t* rawData = nullptr; /* 原始数据域 */

    /* 根据 functionGroup 和 subFunction 解析出的详细内容 */
    bool hasBlockList = false; /* 是否有块列表数据 */
    std::vector<BlockListItem> blockList; /* 块列表数据 */

    bool hasBlockType = false; /* 是否有按类型列出程序块数据 */
    BlockListOfTypeData blockListOfType; /* 按类型列出程序块数据 */

    bool hasBlockInfo = false; /* 是否有块信息数据 */
    BlockInfoData blockInfo; /* 块信息数据 */

    bool hasSzl = false; /* 是否有SZL数据 */
    SzlHeader szlHeader; /* SZL头部 */
    std::vector<SzlData> szlDatas; /* SZL数据列表 */

    bool hasMessageService = false; /* 是否有消息服务数据 */
    MessageServiceData msgService; /* 消息服务数据 */

    bool hasTimestamp = false; /* 是否包含时间戳 */
    S7Timestamp timestamp; /* 时间戳 */
};

/**
 * @brief 读/写变量寻址规范
 */
struct ReadWriteParamItem
{
    uint8_t variableSpecification = 0; /* 变量规范类型, 通常为0x12 */
    uint8_t addressLength = 0; /* 地址长度 */
    uint8_t syntaxId = 0; /* 语法ID, 0x10=S7ANY, 0x82=DBREAD */
    uint8_t transportSize = 0; /* 传输大小 */
    uint16_t length = 0; /* 数据长度(元素个数) */
    uint16_t dbNumber = 0; /* DB块号 */
    uint8_t area = 0; /* 存储区 */
    uint32_t address = 0; /* 字节地址 */
};

/**
 * @brief 读/写参数(0x04,0x05)
 */
struct ReadWriteParam
{
    uint8_t itemCount = 0; /* 条目数 */
    std::vector<ReadWriteParamItem> items; /* 变量条目列表 */
};

/**
 * @brief 读写数据
 */
struct ReadWriteData
{
    uint8_t returnCode = 0; /* 返回码 */
    uint8_t transportSize = 0; /* 传输大小 */
    uint16_t length = 0; /* 数据长度(位为单位时*8) */
    const uint8_t* data = nullptr; /* 实际数据 */
};

/**
 * @brief 下载参数功能状态标志
 */
struct DownloadFunctionStatus
{
    uint8_t moreDataFollowing : 1; /* bit0: 是否有更多数据 */
    uint8_t error : 1; /* bit1: 是否错误 */
};

/**
 * @brief 下载参数文件名(原始值需要转为ASCII字符)
 */
struct DownloadFilename
{
    char fileIdentifier = 0; /* 文件标识符, 5f 表示为 '_', 值定义为: '_'表示完整模块, '$'表示子模块 */
    char blockType[3] = {0}; /* 块类型(字符串), 值定义: 0A=DB, 0B=OB, 0C=FC, 0D=FB, 0E=... */
    char blockNumber[6] = {0}; /* 块号(字符串), 例如: "00001" */
    char destFileSystem = 0; /* 目标文件系统(ASCII), 50 表示为 'P', 值定义为: 'A'表示Active, 'P'表示Passive */
};

/**
 * @brief 请求下载参数(0x1A)
 */
struct RequestDownloadParam
{
    DownloadFunctionStatus funcStatus; /* 功能状态 */
    uint16_t unknownByteInBlockControl1 = 0; /* 未知1 */
    uint32_t unknownByteInBlockControl2 = 0; /* 未知2 */
    uint8_t filenameLen = 0; /* 文件名长度 */
    DownloadFilename filename; /* 文件名 */
    uint8_t lengthPart2 = 0; /* 第二部分长度 */
    char unknownCharBeforeLoadMem = 0; /* 未知字符(ASCII), 31 表示为 '1' */
    char lengthOfLoadMemory[7] = {0}; /* 装载内存大小(十六进制字符串), 例如: "000500" */
    char lengthOfMc7Code[7] = {0}; /* MC7代码大小(十六进制字符串), 例如: "000400" */
};

/**
 * @brief 请求下载响应数据
 */
struct RequestDownloadData
{
};

/**
 * @brief 下载块参数(0x1B)
 */
struct DownloadBlockParam
{
    DownloadFunctionStatus funcStatus; /* 功能状态 */
    uint16_t unknownByteInBlockControl1 = 0; /* 未知1 */
    uint32_t unknownByteInBlockControl2 = 0; /* 未知2 */
    uint8_t filenameLen = 0; /* 文件名长度 */
    DownloadFilename filename; /* 文件名 */
};

/**
 * @brief 下载块数据
 */
struct DownloadBlockData
{
    uint16_t length = 0; /* 数据长度 */
    uint16_t unknownByteInBlockControl = 0; /* 未知 */
    const uint8_t* data = nullptr; /* 数据 */
};

/**
 * @brief 下载结束参数(0x1C)
 */
struct DownloadEndedParam
{
    DownloadFunctionStatus funcStatus; /* 功能状态 */
    uint16_t errorCode = 0; /* 错误码 */
    uint32_t unknownByteInBlockControl = 0; /* 未知 */
    uint8_t filenameLen = 0; /* 文件名长度 */
    DownloadFilename filename; /* 文件名 */
};

/**
 * @brief 下载结束响应数据
 */
struct DownloadEndedData
{
};

/**
 * @brief 开始上传参数(0x1D)
 */
struct StartUploadParam
{
    uint8_t blockTypeLen = 0;
    uint8_t blockNumLen = 0;
    uint8_t fileSystemLen = 0;
    char blockType[3] = {0}; /* 块类型(字符串), 值定义: 0A=DB, 0B=OB, 0C=FC, 0D=FB, 0E=... */
    char blockNumber[6] = {0}; /* 块号(字符串), 例如: "00001" */
    char fileSystem = 0; /* 文件系统(ASCII), 50 表示为 'P', 值定义为: 'A'表示Active, 'P'表示Passive */
};

/**
 * @brief 开始上传响应数据
 */
struct StartUploadData
{
    uint32_t uploadId = 0; /* 上传会话ID(大端) */
    uint8_t status = 0; /* 状态码: 0=成功 */
};

/**
 * @brief 上传块数据参数(0x1E)
 */
struct UploadParam
{
    uint32_t uploadId = 0; /* 上传会话ID(大端) */
};

/**
 * @brief 上传块数据
 */
struct UploadData
{
    uint16_t dataOffset = 0; /* 数据偏移 */
    uint16_t dataLen = 0; /* 数据长度 */
    const uint8_t* blockData = nullptr; /* 块原始数据 */
    bool isLastBlock = false; /* 是否最后一块数据 */
};

/**
 * @brief 上传结束参数(0x1F)
 */
struct EndUploadParam
{
    uint32_t uploadId = 0; /* 上传会话ID (大端) */
};

/**
 * @brief 上传结束响应数据
 */
struct EndUploadData
{
    uint8_t returnCode = 0; /* 返回码 */
    uint8_t errorClass = 0; /* 错误类型 */
    uint8_t errorCode = 0; /* 错误码 */
};

/**
 * @brief PLC控制参数(0x26)
 */
struct PlcControlParam
{
    uint8_t paramCount = 0; /* 参数字符串数量 */
    std::vector<std::string> params; /* 参数列表, 如"P_PROGRAM" */
};

/**
 * @brief PLC控制响应数据
 */
struct PlcControlData
{
    uint8_t returnCode = 0; /* 返回码 */
    std::string statusMsg; /* 状态信息 */
};

/**
 * @brief PI服务文件名
 */
struct PiServiceFilename
{
    char blockType[3] = {0}; /* 块类型(字符串), 值定义: 0A=DB, 0B=OB, 0C=FC, 0D=FB, 0E=... */
    char blockNumber[6] = {0}; /* 块号(字符串), 例如: "00001" */
    char destFileSystem = 0; /* 目标文件系统(ASCII), 50 表示为 'P', 值定义为: 'A'表示Active, 'P'表示Passive */
};

/**
 * @brief PI服务参数块
 */
struct PiServiceParameterBlock
{
    uint8_t numberOfBlocks = 0; /* 块数量 */
    uint8_t unknownByte = 0; /* 未知字节 */
    PiServiceFilename filename; /* 文件名 */
};

/**
 * @brief PI服务参数(0x28)
 */
struct PiServiceParam
{
    uint64_t unknownBytes = 0; /* 未知字节 */
    uint16_t paramBlockLength = 0; /* 参数块长度 */
    PiServiceParameterBlock paramBlock; /* 参数块 */
    uint8_t serviceNameLength = 0; /* 服务名长度 */
    char serviceName[32] = {0}; /* 服务名(字符串), 如: "_INSE", "_MODU"等 */
};

/**
 * @brief PI服务响应数据
 */
struct PiServiceData
{
    uint8_t returnCode = 0; /* 返回码 */
    uint16_t dataLen = 0; /* 数据长度 */
    const uint8_t* responseData = nullptr; /* 响应数据 */
};

/**
 * @brief PLC停止参数(0x29)
 */
struct PlcStopParam
{
};

/**
 * @brief PLC停止响应数据
 */
struct PlcStopData
{
    uint8_t returnCode = 0; /* 返回码 */
    uint8_t stopStatus = 0; /* 停止状态: 0=成功停止 */
};

/**
 * @brief 拷贝RAM到ROM参数(0x2A)
 */
struct CopyRamToRomParam
{
    uint8_t paramCount = 0; /* 参数个数 */
    char blockType[3] = {0}; /* 块类型(字符串), 值定义: 0A=DB, 0B=OB, 0C=FC, 0D=FB, 0E=... */
    char blockNumber[6] = {0}; /* 块号(字符串), 例如: "00001" */
};

/**
 * @brief 拷贝RAM到ROM响应数据
 */
struct CopyRamToRomData
{
    uint8_t returnCode = 0; /* 返回码 */
    uint8_t copyStatus = 0; /* 拷贝状态: 0=成功 */
};

/**
 * @brief 压缩内存参数(0x2B)
 */
struct CompressParam
{
    uint8_t memoryType = 0; /* 内存类型: 0=Load memory, 1=Work memory */
    uint8_t reserved = 0; /* 保留字段 */
};

/**
 * @brief 压缩内存响应数据
 */
struct CompressData
{
    uint8_t returnCode = 0; /* 返回码 */
    uint8_t compressStatus = 0; /* 压缩状态: 0=成功 */
    uint32_t freedBytes = 0; /* 释放的字节数 */
};

/**
 * @brief 块操作参数(0x2C/0x2D/0x2E等)
 */
struct BlockOperationParam
{
    uint8_t paramCount = 0;
    char blockType[3] = {0}; /* 块类型(字符串), 值定义: 0A=DB, 0B=OB, 0C=FC, 0D=FB, 0E=... */
    char blockNumber[6] = {0}; /* 块号(字符串), 例如: "00001" */
    char fileSystem = 0; /* 文件系统(ASCII), 50 表示为 'P', 值定义为: 'A'表示Active, 'P'表示Passive */
};

/**
 * @brief 块操作响应数据(删除/替换/状态查询)
 */
struct BlockOperationData
{
    uint8_t returnCode = 0; /* 返回码 */
    uint8_t blockStatus = 0; /* 块状态(仅查询有效): 0=存在, 1=不存在 */
    uint8_t errorClass = 0; /* 错误类型 */
    uint8_t errorCode = 0; /* 错误码 */
};

/**
 * @brief 通信协商参数(0xF0)
 */
struct SetupCommParam
{
    uint8_t reserved = 0;
    uint16_t maxAmqCalling = 0; /* 请求方最大并行作业数 */
    uint16_t maxAmqCalled = 0; /* 响应方最大并行作业数 */
    uint16_t pduLength = 0; /* 协商PDU长度 */
};

/**
 * @brief 通信协商响应数据
 */
struct SetupCommData
{
    uint8_t returnCode = 0; /* 返回码 */
    uint16_t maxAmqCaller = 0; /* 协商后请求方最大并行作业数 */
    uint16_t maxAmqCallee = 0; /* 协商后响应方最大并行作业数 */
    uint16_t pduLength = 0; /* 协商后PDU长度 */
};

/**
 * @brief S7COMM完整信息
 */
struct S7CommInfo
{
    S7Header header; /* 头部 */
    const uint8_t* rawParam = nullptr; /* 原始参数域 */
    const uint8_t* rawData = nullptr; /* 原始数据域 */
    uint8_t funcCode = 0; /* 当前功能码 */

    CpuServiceParam cpuParam; /* CPU服务(0x00) */
    CpuServiceData cpuData; /* CPU服务数据 */

    ReadWriteParam rwParam; /* 读/写参数(0x04/0x05) */
    std::vector<ReadWriteData> rwData; /* 读/写数据 */

    RequestDownloadParam reqDownloadParam; /* 请求下载(0x1A) */
    RequestDownloadData reqDownloadData; /* 请求下载响应数据 */

    DownloadBlockParam downloadBlockParam; /* 下载块(0x1B) */
    DownloadBlockData downloadBlockData; /* 下载块数据 */

    DownloadEndedParam downloadEndedParam; /* 下载结束(0x1C) */
    DownloadEndedData downloadEndedData; /* 下载结束响应数据 */

    StartUploadParam startUploadParam; /* 开始上传(0x1D) */
    StartUploadData startUploadData; /* 开始上传响应数据 */

    UploadParam uploadParam; /* 上传(0x1E) */
    UploadData uploadData; /* 上传块数据 */

    EndUploadParam endUploadParam; /* 结束上传(0x1F) */
    EndUploadData endUploadData; /* 上传结束响应数据 */

    PlcControlParam plcCtrlParam; /* PLC控制(0x26) */
    PlcControlData plcCtrlData; /* PLC控制响应数据 */

    PiServiceParam piParam; /* PI服务(0x28) */
    PiServiceData piData; /* PI服务响应数据 */

    PlcStopParam plcStopParam; /* PLC停止(0x29) */
    PlcStopData plcStopData; /* PLC停止响应数据 */

    CopyRamToRomParam copyRamToRomParam; /* 拷贝RAM到ROM(0x2A) */
    CopyRamToRomData copyRamToRomData; /* 拷贝RAM到ROM响应数据 */

    CompressParam compressParam; /* 压缩内存(0x2B) */
    CompressData compressData; /* 压缩内存响应数据 */

    BlockOperationParam blockOpParam; /* 块操作(0x2C-0x2E) */
    BlockOperationData blockOpData; /* 块操作响应数据 */

    SetupCommParam setupCommParam; /* 通信协商(0xF0) */
    SetupCommData setupCommData; /* 通信协商响应数据 */

    std::vector<uint8_t> reassembledData; /* 重组后的数据(仅S7COMM分片时使用) */
};
} // namespace s7comm
} // namespace npacket
