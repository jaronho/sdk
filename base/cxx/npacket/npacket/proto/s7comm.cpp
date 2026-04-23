#include "s7comm.h"

#include <string.h>

namespace npacket
{
bool S7CommParser::parse(size_t flag, size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                         const ProtocolHeader* header, const TpktInfo& tpktInfo, const CotpInfo& cotpInfo, const uint8_t* payload,
                         uint32_t payloadLen)
{
    if (!payload || 0 == payloadLen)
    {
        return false;
    }
    if (0x32 != payload[0]) /* S7COMM协议标识: 0x32 */
    {
        return false;
    }
    s7comm::S7CommInfo s7commInfo;
    if (!parseS7CommInfo(payload, payloadLen, s7commInfo))
    {
        return false;
    }
    if (m_frameCb)
    {
        m_frameCb(ntp, totalLen, header, tpktInfo, cotpInfo, s7commInfo);
    }
    return true;
}

void S7CommParser::setFrameCallback(const FRAME_CALLBACK& callback)
{
    m_frameCb = callback;
}

bool S7CommParser::parseS7CommInfo(const uint8_t* data, uint32_t dataLen, s7comm::S7CommInfo& info)
{
    memset(&info, 0, sizeof(s7comm::S7CommInfo));
    if (dataLen < 10)
    {
        return false;
    }
    const uint8_t* buffer = data;
    uint32_t bufferLen = dataLen;
    /* 解析S7COMM头部 */
    uint32_t headerLen = 0;
    if (!parseS7Header(buffer, bufferLen, info.header, headerLen))
    {
        return false;
    }
    buffer += headerLen;
    bufferLen -= headerLen;
    /* 校验参数+数据长度 */
    uint32_t expectedLen = info.header.parameterLength + info.header.dataLength;
    if (bufferLen < expectedLen)
    {
        return false;
    }
    /* 解析参数域 */
    if (info.header.parameterLength > 0)
    {
        const uint8_t* parameterPtr = buffer;
        info.rawParameter = parameterPtr;
        buffer += info.header.parameterLength;
        bufferLen -= info.header.parameterLength;
        /* 解析参数 */
        info.functionCode = parameterPtr[0];
        switch ((s7comm::FunctionCode)info.functionCode)
        {
        case s7comm::FunctionCode::CPU_SERVICES:
            break;
        case s7comm::FunctionCode::READ_VARIABLE:
        case s7comm::FunctionCode::WRITE_VARIABLE:
            parseReadWriteParam(parameterPtr, info.header.parameterLength, info.rwParam);
            break;
        case s7comm::FunctionCode::REQUEST_DOWNLOAD:
            parseRequestDownloadParam(parameterPtr, info.header.parameterLength, info.reqDlParam);
            break;
        case s7comm::FunctionCode::DOWNLOAD_BLOCK:
            break;
        case s7comm::FunctionCode::DOWNLOAD_ENDED:
            break;
        case s7comm::FunctionCode::START_UPLOAD:
            parseStartUploadParam(parameterPtr, info.header.parameterLength, info.startUlParam);
            break;
        case s7comm::FunctionCode::UPLOAD:
            parseUploadParam(parameterPtr, info.header.parameterLength, info.uploadParam);
            break;
        case s7comm::FunctionCode::END_UPLOAD:
            parseEndUploadParam(parameterPtr, info.header.parameterLength, info.endUlParam);
            break;
        case s7comm::FunctionCode::PLC_CONTROL:
            parsePlcControlParam(parameterPtr, info.header.parameterLength, info.plcCtrlParam);
            break;
        case s7comm::FunctionCode::PI_SERVICE:
            parsePiServiceParam(parameterPtr, info.header.parameterLength, info.piParam);
            break;
        case s7comm::FunctionCode::PLC_STOP:
            parsePlcStopParam(parameterPtr, info.header.parameterLength, info.plcStopParam);
            break;
        case s7comm::FunctionCode::COPY_RAM_TO_ROM:
            break;
        case s7comm::FunctionCode::COMPRESS:
            break;
        case s7comm::FunctionCode::DELETE_BLOCK:
        case s7comm::FunctionCode::REPLACE_BLOCK:
        case s7comm::FunctionCode::BLOCK_STATUS:
            parseBlockOperationParam(parameterPtr, info.header.parameterLength, info.blockOpParam);
            break;
        case s7comm::FunctionCode::SETUP_COMMUNICATION:
            parseSetupCommParam(parameterPtr, info.header.parameterLength, info.setupParam);
            break;
        default: /* 0x00/0x1B/0x1C/0x2A/0x2B等保留原始数据, 供上层自行处理 */
            break;
        }
    }
    /* 解析数据域 */
    if (info.header.dataLength > 0)
    {
        const uint8_t* dataPtr = buffer;
        info.rawData = dataPtr;
        buffer += info.header.dataLength;
        bufferLen -= info.header.dataLength;
        /* 解析数据 */
        switch ((s7comm::FunctionCode)info.functionCode)
        {
        case s7comm::FunctionCode::CPU_SERVICES:
            break;
        case s7comm::FunctionCode::READ_VARIABLE: /* 读响应: 数据项列表 */
        case s7comm::FunctionCode::WRITE_VARIABLE: /* 写请求: 数据项列表(格式与读响应相同) */
            if (info.rwParam.itemCount > 0)
            {
                parseDataItems(dataPtr, info.header.dataLength, info.rwParam.itemCount, info.dataItems);
            }
            break;
        case s7comm::FunctionCode::REQUEST_DOWNLOAD:
            break;
        case s7comm::FunctionCode::DOWNLOAD_BLOCK:
            break;
        case s7comm::FunctionCode::DOWNLOAD_ENDED:
            break;
        case s7comm::FunctionCode::START_UPLOAD:
            break;
        case s7comm::FunctionCode::UPLOAD: /* 上传响应: 块原始数据, 通过 awData访问 */
            break;
        case s7comm::FunctionCode::END_UPLOAD:
            break;
        case s7comm::FunctionCode::PLC_CONTROL:
            break;
        case s7comm::FunctionCode::PI_SERVICE:
            break;
        case s7comm::FunctionCode::PLC_STOP:
            break;
        case s7comm::FunctionCode::COPY_RAM_TO_ROM:
            break;
        case s7comm::FunctionCode::COMPRESS:
            break;
        case s7comm::FunctionCode::DELETE_BLOCK:
            break;
        case s7comm::FunctionCode::REPLACE_BLOCK:
            break;
        case s7comm::FunctionCode::BLOCK_STATUS:
            break;
        case s7comm::FunctionCode::SETUP_COMMUNICATION:
            break;
        default:
            break;
        }
    }
    return true;
}

bool S7CommParser::parseS7Header(const uint8_t* data, uint32_t dataLen, s7comm::S7Header& header, uint32_t& headerLen)
{
    if (dataLen < 10)
    {
        return false;
    }
    header.protocolId = data[0];
    if (0x32 != header.protocolId) /* 不是S7COMM协议 */
    {
        return false;
    }
    header.rosctr = (s7comm::RosctrType)data[1];
    header.redundancyIdentification = (data[2] << 8) | data[3];
    header.protocolDataUnitReference = (data[4] << 8) | data[5];
    header.parameterLength = (data[6] << 8) | data[7];
    header.dataLength = (data[8] << 8) | data[9];
    /* ACK_DATA类型有额外的错误码字段 */
    if (s7comm::RosctrType::ACK_DATA == header.rosctr)
    {
        if (dataLen < 12)
        {
            return false;
        }
        header.errorClass = data[10];
        header.errorCode = data[11];
        headerLen = 12;
    }
    else
    {
        header.errorClass = 0;
        header.errorCode = 0;
        headerLen = 10;
    }
    /* 校验ROSCTR */
    switch (header.rosctr)
    {
    case s7comm::RosctrType::JOB:
    case s7comm::RosctrType::ACK:
    case s7comm::RosctrType::ACK_DATA:
    case s7comm::RosctrType::USERDATA:
        break;
    default:
        return false;
    }
    return true;
}

bool S7CommParser::parseItemSpec(const uint8_t* data, uint32_t dataLen, s7comm::ItemSpec& item, uint32_t& itemLen)
{
    itemLen = 0;
    if (dataLen < 4)
    {
        return false;
    }
    item.variableSpecification = data[0];
    item.addressLength = data[1];
    if (0x12 == item.variableSpecification && 0x0A == item.addressLength && dataLen >= 12) /* S7ANY格式: 0x10, 标准12字节 */
    {
        item.syntaxId = data[2];
        item.transportSize = data[3];
        item.length = (data[4] << 8) | data[5];
        item.dbNumber = (data[6] << 8) | data[7];
        item.area = data[8];
        item.address[0] = data[9];
        item.address[1] = data[10];
        item.address[2] = data[11];
        /* 计算实际地址: (address[0] << 16) | (address[1] << 8) | address[2], 单位为bit */
        itemLen = 12;
        return true;
    }
    else if (0x82 == item.syntaxId) /* DBREAD格式: 0x82, 或优化访问格式等 */
    {
        /* 简化处理, 实际需要根据具体格式调整 */
        itemLen = 2 + item.addressLength;
        if (dataLen < itemLen)
        {
            return false;
        }
        return true;
    }
    return false;
}

bool S7CommParser::parseReadWriteParam(const uint8_t* data, uint32_t dataLen, s7comm::ReadWriteParam& param)
{
    if (dataLen < 2)
    {
        return false;
    }
    param.functionCode = data[0];
    param.itemCount = data[1];
    if (0 == param.itemCount)
    {
        return true;
    }
    uint32_t offset = 2;
    for (uint8_t i = 0; i < param.itemCount; ++i)
    {
        s7comm::ItemSpec item;
        uint32_t itemLen = 0;
        if (!parseItemSpec(data + offset, dataLen - offset, item, itemLen))
        {
            return false;
        }
        param.items.emplace_back(item);
        offset += itemLen;
    }
    return true;
}

bool S7CommParser::parseRequestDownloadParam(const uint8_t* data, uint32_t dataLen, s7comm::RequestDownloadParam& param)
{
    if (dataLen < 14)
    {
        return false;
    }
    param.functionCode = data[0];
    param.unknown1 = data[1];
    param.unknown2 = data[2];
    param.blockTypeLen = data[3];
    param.blockNumLen = data[4];
    param.fileSystemLen = data[5];
    uint32_t offset = 6;
    if (2 == param.blockTypeLen && offset + 2 <= dataLen)
    {
        param.blockType = (data[offset] << 8) | data[offset + 1];
        offset += 2;
    }
    if (param.blockNumLen <= 5 && offset + param.blockNumLen <= dataLen)
    {
        memcpy(param.blockNumber, data + offset, param.blockNumLen);
        param.blockNumber[param.blockNumLen] = '\0';
        offset += param.blockNumLen;
    }
    if (1 == param.fileSystemLen && offset < dataLen)
    {
        param.fileSystem = data[offset];
    }
    return true;
}

bool S7CommParser::parseStartUploadParam(const uint8_t* data, uint32_t dataLen, s7comm::StartUploadParam& param)
{
    if (dataLen < 4)
    {
        return false;
    }
    param.functionCode = data[0];
    param.blockTypeLen = data[1];
    param.blockNumLen = data[2];
    param.fileSystemLen = data[3];
    uint32_t offset = 4;
    if (2 == param.blockTypeLen && offset + 2 <= dataLen)
    {
        param.blockType = (data[offset] << 8) | data[offset + 1];
        offset += 2;
    }
    if (param.blockNumLen <= 5 && offset + param.blockNumLen <= dataLen)
    {
        memcpy(param.blockNumber, data + offset, param.blockNumLen);
        param.blockNumber[param.blockNumLen] = '\0';
        offset += param.blockNumLen;
    }
    if (1 == param.fileSystemLen && offset < dataLen)
    {
        param.fileSystem = data[offset];
    }
    return true;
}

bool S7CommParser::parseUploadParam(const uint8_t* data, uint32_t dataLen, s7comm::UploadParam& param)
{
    if (dataLen < 5)
    {
        return false;
    }
    param.functionCode = data[0];
    param.uploadId = ((uint32_t)data[1] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 8) | (uint32_t)data[4];
    return true;
}

bool S7CommParser::parseEndUploadParam(const uint8_t* data, uint32_t dataLen, s7comm::EndUploadParam& param)
{
    if (dataLen < 5)
    {
        return false;
    }
    param.functionCode = data[0];
    param.uploadId = ((uint32_t)data[1] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 8) | (uint32_t)data[4];
    return true;
}

bool S7CommParser::parsePlcControlParam(const uint8_t* data, uint32_t dataLen, s7comm::PlcControlParam& param)
{
    if (dataLen < 2)
    {
        return false;
    }
    param.functionCode = data[0];
    param.paramCount = data[1];
    uint32_t offset = 2;
    for (uint8_t i = 0; i < param.paramCount && offset < dataLen; ++i)
    {
        if (offset >= dataLen)
        {
            break;
        }
        uint8_t strLen = data[offset];
        offset++;
        if (offset + strLen > dataLen)
        {
            return false;
        }
        param.params.emplace_back((const char*)(data + offset), strLen);
        offset += strLen;
    }
    return true;
}

bool S7CommParser::parsePiServiceParam(const uint8_t* data, uint32_t dataLen, s7comm::PiServiceParam& param)
{
    if (dataLen < 2)
    {
        return false;
    }
    param.functionCode = data[0];
    param.paramCount = data[1];
    uint32_t offset = 2;
    for (uint8_t i = 0; i < param.paramCount && offset < dataLen; ++i)
    {
        if (offset >= dataLen)
        {
            break;
        }
        uint8_t strLen = data[offset];
        offset++;
        if (offset + strLen > dataLen)
        {
            return false;
        }
        param.params.emplace_back((const char*)(data + offset), strLen);
        offset += strLen;
    }
    return true;
}

bool S7CommParser::parsePlcStopParam(const uint8_t* data, uint32_t dataLen, s7comm::PlcStopParam& param)
{
    if (dataLen < 1)
    {
        return false;
    }
    param.functionCode = data[0];
    return true;
}

bool S7CommParser::parseBlockOperationParam(const uint8_t* data, uint32_t dataLen, s7comm::BlockOperationParam& param)
{
    if (dataLen < 2)
    {
        return false;
    }
    param.functionCode = data[0];
    param.paramCount = data[1];
    uint32_t offset = 2;
    for (uint8_t i = 0; i < param.paramCount && offset < dataLen; ++i)
    {
        if (offset >= dataLen)
        {
            break;
        }
        uint8_t paramLen = data[offset];
        offset++;
        if (offset + paramLen > dataLen)
        {
            return false;
        }
        /* 标准块操作参数: 2字节类型 + 5字节编号 + 1字节文件系统 */
        if (0 == i && paramLen >= 8)
        {
            param.blockType = (data[offset] << 8) | data[offset + 1];
            memcpy(param.blockNumber, data + offset + 2, 5);
            param.blockNumber[5] = '\0';
            param.fileSystem = data[offset + 7];
        }
        offset += paramLen;
    }
    return true;
}

bool S7CommParser::parseSetupCommParam(const uint8_t* data, uint32_t dataLen, s7comm::SetupCommParam& param)
{
    if (dataLen < 8)
    {
        return false;
    }
    param.functionCode = data[0];
    param.reserved = data[1];
    param.maxAmqCaller = (data[2] << 8) | data[3];
    param.maxAmqCallee = (data[4] << 8) | data[5];
    param.pduLength = (data[6] << 8) | data[7];
    return true;
}

bool S7CommParser::parseDataItem(const uint8_t* data, uint32_t dataLen, s7comm::DataItem& item, uint32_t& itemLen)
{
    itemLen = 0;
    if (dataLen < 4)
    {
        return false;
    }
    item.returnCode = data[0];
    item.transportSize = data[1];
    /* 数据长度处理: 如果transportSize是bit类型(0x01, 0x03), length是bit数, 需要/8向上取整 */
    uint16_t lenRaw = (data[2] << 8) | data[3];
    /* 判断是否为bit类型 */
    bool isBitType = (0x01 == item.transportSize || 0x03 == item.transportSize);
    if (isBitType)
    {
        item.length = (lenRaw + 7) / 8; /* bit转byte, 向上取整 */
    }
    else
    {
        item.length = lenRaw;
    }
    uint32_t totalLen = 4 + item.length;
    /* 如果数据长度为奇数, 西门子会填充1字节使其为偶数 */
    if (0 != item.length % 2)
    {
        totalLen += 1;
    }
    if (dataLen < totalLen)
    {
        return false;
    }
    item.data = data + 4;
    itemLen = totalLen;
    return true;
}

bool S7CommParser::parseDataItems(const uint8_t* data, uint32_t dataLen, uint8_t itemCount, std::vector<s7comm::DataItem>& items)
{
    uint32_t offset = 0;
    for (uint8_t i = 0; i < itemCount; ++i)
    {
        s7comm::DataItem item;
        uint32_t itemLen = 0;
        if (!parseDataItem(data + offset, dataLen - offset, item, itemLen))
        {
            return false;
        }
        items.emplace_back(item);
        offset += itemLen;
    }
    return true;
}
} // namespace npacket
