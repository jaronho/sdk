#include "s7comm.h"

#include <string.h>

namespace npacket
{
inline void parseAsciiString(const uint8_t* data, uint32_t maxLen, std::string& str)
{
    uint32_t actualLen = 0;
    for (uint32_t i = 0; i < maxLen; ++i)
    {
        if (0x00 == data[i])
        {
            actualLen = i;
            break;
        }
    }
    str.clear();
    if (actualLen > 0)
    {
        str.assign((const char*)data, actualLen);
    }
}

S7CommParser::S7CommParser(size_t fragTimeout) : m_fragTimeout((fragTimeout < 1000 || fragTimeout > 300000) ? 30000 : fragTimeout) {}

bool S7CommParser::parse(size_t flag, size_t num, const std::chrono::steady_clock::time_point& ntp, uint32_t totalLen,
                         const ProtocolHeader* header, const TpktInfo& tpktInfo, const CotpInfo& cotpInfo, const uint8_t* payload,
                         uint32_t payloadLen)
{
    cleanupFragmentCache(ntp); /* 清理超时分片缓存 */
    if (!payload || 0 == payloadLen)
    {
        return false;
    }
    if (0x32 != payload[0]) /* S7COMM协议标识: 0x32 */
    {
        return false;
    }
    s7comm::S7CommInfo s7Info;
    if (!parseS7CommInfo(ntp, header, payload, payloadLen, s7Info))
    {
        return false;
    }
    /* 如果是USERDATA CPU services的分片(非最后一片), 暂不上报 */
    if (s7comm::RosctrType::USERDATA == s7Info.header.rosctr && !s7Info.cpuParam.items.empty())
    {
        const auto& item = s7Info.cpuParam.items[0];
        if (0x12 == item.syntaxId && 0x01 == item.lastDataUnit)
        {
            return true; /* 已缓存, 等待后续分片 */
        }
    }
    if (m_frameCb)
    {
        m_frameCb(ntp, totalLen, header, tpktInfo, cotpInfo, s7Info);
    }
    return true;
}

void S7CommParser::setFrameCallback(const FRAME_CALLBACK& callback)
{
    m_frameCb = callback;
}

void S7CommParser::cleanupFragmentCache(const std::chrono::steady_clock::time_point& ntp)
{
    for (auto iter = m_cpuServiceFragments.begin(); m_cpuServiceFragments.end() != iter;)
    {
        if (ntp - iter->second.lastAccess > std::chrono::milliseconds(m_fragTimeout))
        {
            iter = m_cpuServiceFragments.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

bool S7CommParser::parseS7CommInfo(const std::chrono::steady_clock::time_point& ntp, const ProtocolHeader* header, const uint8_t* data,
                                   uint32_t dataLen, s7comm::S7CommInfo& info)
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
        info.rawParam = parameterPtr;
        buffer += info.header.parameterLength;
        bufferLen -= info.header.parameterLength;
        /* 解析参数 */
        info.funcCode = parameterPtr[0];
        switch ((s7comm::FunctionCode)info.funcCode)
        {
        case s7comm::FunctionCode::CPU_SERVICES:
            parseCpuServiceParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.cpuParam);
            break;
        case s7comm::FunctionCode::READ_VARIABLE:
        case s7comm::FunctionCode::WRITE_VARIABLE:
            parseReadWriteParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.rwParam);
            break;
        case s7comm::FunctionCode::REQUEST_DOWNLOAD:
            parseRequestDownloadParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.reqDownloadParam);
            break;
        case s7comm::FunctionCode::DOWNLOAD_BLOCK:
            parseDownloadBlockParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.downloadBlockParam);
            break;
        case s7comm::FunctionCode::DOWNLOAD_ENDED:
            parseDownloadEndedParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.downloadEndedParam);
            break;
        case s7comm::FunctionCode::START_UPLOAD:
            parseStartUploadParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.startUploadParam);
            break;
        case s7comm::FunctionCode::UPLOAD_BLOCK:
            parseUploadParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.uploadParam);
            break;
        case s7comm::FunctionCode::END_UPLOAD:
            parseEndUploadParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.endUploadParam);
            break;
        case s7comm::FunctionCode::PLC_CONTROL:
            parsePlcControlParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.plcCtrlParam);
            break;
        case s7comm::FunctionCode::PI_SERVICE:
            parsePiServiceParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.piParam);
            break;
        case s7comm::FunctionCode::PLC_STOP:
            parsePlcStopParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.plcStopParam);
            break;
        case s7comm::FunctionCode::COPY_RAM_TO_ROM:
            parseCopyRamToRomParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.copyRamToRomParam);
            break;
        case s7comm::FunctionCode::COMPRESS:
            parseCompressParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.compressParam);
            break;
        case s7comm::FunctionCode::DELETE_BLOCK:
        case s7comm::FunctionCode::REPLACE_BLOCK:
        case s7comm::FunctionCode::BLOCK_STATUS:
            parseBlockOperationParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.blockOpParam);
            break;
        case s7comm::FunctionCode::SETUP_COMMUNICATION:
            parseSetupCommParam(info.header.rosctr, parameterPtr, info.header.parameterLength, info.setupCommParam);
            break;
        default:
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
        switch ((s7comm::FunctionCode)info.funcCode)
        {
        case s7comm::FunctionCode::CPU_SERVICES:
            parseCpuServiceData(info.header.rosctr, dataPtr, info.header.dataLength, info.cpuData);
            if (s7comm::RosctrType::USERDATA == info.header.rosctr && !info.cpuParam.items.empty()) /* 对USERDATA类型进行分片重组 */
            {
                const auto& item = info.cpuParam.items[0];
                if (0x12 == item.syntaxId && !tryReassembleCpuServiceData(ntp, header, info)) /* 分片未完整, 暂不解析详细内容 */
                {
                    break;
                }
            }
            parseCpuServiceDataDetail(info.cpuParam, info.cpuData); /* 根据参数解析详细内容 */
            break;
        case s7comm::FunctionCode::READ_VARIABLE: /* 读响应: 数据项列表 */
        case s7comm::FunctionCode::WRITE_VARIABLE: /* 写请求: 数据项列表 */
            parseReadWriteData(info.header.rosctr, (s7comm::FunctionCode)info.funcCode, dataPtr, info.header.dataLength,
                               info.rwParam.itemCount, info.rwData);
            break;
        case s7comm::FunctionCode::REQUEST_DOWNLOAD:
            parseRequestDownloadData(info.header.rosctr, dataPtr, info.header.dataLength, info.reqDownloadData);
            break;
        case s7comm::FunctionCode::DOWNLOAD_BLOCK:
            parseDownloadBlockData(info.header.rosctr, dataPtr, info.header.dataLength, info.downloadBlockData);
            break;
        case s7comm::FunctionCode::DOWNLOAD_ENDED:
            parseDownloadEndedData(info.header.rosctr, dataPtr, info.header.dataLength, info.downloadEndedData);
            break;
        case s7comm::FunctionCode::START_UPLOAD:
            parseStartUploadData(info.header.rosctr, dataPtr, info.header.dataLength, info.startUploadData);
            break;
        case s7comm::FunctionCode::UPLOAD_BLOCK:
            parseUploadData(info.header.rosctr, dataPtr, info.header.dataLength, info.uploadData);
            break;
        case s7comm::FunctionCode::END_UPLOAD:
            parseEndUploadData(info.header.rosctr, dataPtr, info.header.dataLength, info.endUploadData);
            break;
        case s7comm::FunctionCode::PLC_CONTROL:
            parsePlcControlData(info.header.rosctr, dataPtr, info.header.dataLength, info.plcCtrlData);
            break;
        case s7comm::FunctionCode::PI_SERVICE:
            parsePiServiceData(info.header.rosctr, dataPtr, info.header.dataLength, info.piData);
            break;
        case s7comm::FunctionCode::PLC_STOP:
            parsePlcStopData(info.header.rosctr, dataPtr, info.header.dataLength, info.plcStopData);
            break;
        case s7comm::FunctionCode::COPY_RAM_TO_ROM:
            parseCopyRamToRomData(info.header.rosctr, dataPtr, info.header.dataLength, info.copyRamToRomData);
            break;
        case s7comm::FunctionCode::COMPRESS:
            parseCompressData(info.header.rosctr, dataPtr, info.header.dataLength, info.compressData);
            break;
        case s7comm::FunctionCode::DELETE_BLOCK:
        case s7comm::FunctionCode::REPLACE_BLOCK:
        case s7comm::FunctionCode::BLOCK_STATUS:
            parseBlockOperationData(info.header.rosctr, dataPtr, info.header.dataLength, info.blockOpData);
            break;
        case s7comm::FunctionCode::SETUP_COMMUNICATION:
            parseSetupCommData(info.header.rosctr, dataPtr, info.header.dataLength, info.setupCommData);
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
    header.redundancyIdentification = ((uint16_t)data[2] << 8) | data[3];
    header.protocolDataUnitReference = ((uint16_t)data[4] << 8) | data[5];
    header.parameterLength = ((uint16_t)data[6] << 8) | data[7];
    header.dataLength = ((uint16_t)data[8] << 8) | data[9];
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

bool S7CommParser::parseCpuServiceParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                        s7comm::CpuServiceParam& param)
{
    if (dataLen < 2)
    {
        return false;
    }
    param.itemCount = data[1];
    if (0 == param.itemCount)
    {
        return true;
    }
    uint32_t offset = 2;
    for (uint8_t i = 0; i < param.itemCount && offset < dataLen; ++i)
    {
        s7comm::CpuServiceParamItem item;
        if (offset >= dataLen)
        {
            return false;
        }
        item.variableSpecification = data[offset];
        offset++;
        if (offset >= dataLen)
        {
            return false;
        }
        item.lengthOfFollowingAddressSpecification = data[offset];
        offset++;
        if (offset >= dataLen)
        {
            return false;
        }
        item.syntaxId = data[offset];
        offset++;
        if (offset + 2 >= dataLen)
        {
            return false;
        }
        item.type = (data[offset] >> 6) & 0x03; /* 高2位 */
        item.functionGroup = data[offset] & 0x3F; /* 低6位 */
        offset++;
        item.subFunction = data[offset];
        offset++;
        if (offset >= dataLen)
        {
            return false;
        }
        item.sequenceNumber = data[offset];
        offset++;
        if (0x12 == item.syntaxId && offset + 4 <= dataLen)
        {
            item.dataUnitReferenceNumber = data[offset];
            offset++;
            item.lastDataUnit = data[offset];
            offset++;
            item.errorCode = ((uint16_t)data[offset] << 8) | data[offset + 1];
            offset += 2;
        }
        param.items.emplace_back(item);
    }
    return true;
}

bool S7CommParser::parseCpuServiceBlockListData(const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceData& dataOut)
{
    if (dataLen < 4 || 0 != dataLen % 4)
    {
        return false; /* 每条目固定4字节, 总长度必须是4的倍数 */
    }
    dataOut.hasBlockList = true;
    uint32_t itemCount = dataLen / 4;
    for (uint32_t i = 0; i < itemCount; ++i)
    {
        uint32_t offset = i * 4;
        s7comm::BlockListItem item;
        /* Block type: 2字节ASCII十六进制字符串, 如"0D" */
        char typeStr[3] = {(char)data[offset], (char)data[offset + 1], '\0'};
        item.blockType = (uint8_t)strtoul(typeStr, nullptr, 16);
        /* Block count: 2字节大端uint16_t */
        item.blockCount = ((uint16_t)data[offset + 2] << 8) | data[offset + 3];
        dataOut.blockList.emplace_back(item);
    }
    return true;
}

bool S7CommParser::parseCpuServiceBlockListOfTypeData(bool isRequest, const uint8_t* data, uint32_t dataLen,
                                                      s7comm::CpuServiceData& dataOut)
{
    if (isRequest)
    {
        if (dataLen < 2)
        {
            return false;
        }
        dataOut.hasBlockType = true;
        dataOut.blockListOfType.isRequest = isRequest;
        /* 块类型: 第1-2字节为ASCII十六进制字符串, 如"0B" */
        char typeStr[3] = {(char)data[0], (char)data[1], '\0'};
        dataOut.blockListOfType.req.blockType = (uint8_t)strtoul(typeStr, nullptr, 16);
    }
    else
    {
        if (dataLen < 4 || 0 != dataLen % 4)
        {
            return false; /* 每条目固定4字节, 总长度必须是4的倍数 */
        }
        dataOut.hasBlockType = true;
        dataOut.blockListOfType.isRequest = isRequest;
        uint32_t itemCount = dataLen / 4;
        for (uint32_t i = 0; i < itemCount; ++i)
        {
            uint32_t offset = i * 4;
            s7comm::BlockListOfTypeItem item;
            item.blockNumber = ((uint16_t)data[offset] << 8) | data[offset + 1];
            item.blockFlags = data[offset + 2];
            item.blockLanguage = data[offset + 3];
            dataOut.blockListOfType.resp.items.emplace_back(item);
        }
    }
    return true;
}

bool S7CommParser::parseCpuServiceBlockInfoData(bool isRequest, const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceData& dataOut)
{
    if (isRequest)
    {
        if (dataLen < 8)
        {
            return false;
        }
        dataOut.hasBlockInfo = true;
        dataOut.blockInfo.isRequest = isRequest;
        /* 块类型: 第1-2字节为ASCII十六进制字符串, 如"0B" */
        char typeStr[3] = {(char)data[0], (char)data[1], '\0'};
        dataOut.blockInfo.req.blockType = (uint8_t)strtoul(typeStr, nullptr, 16);
        /* 块号: 第3-7字节为ASCII十进制数字字符串, 如"32768" */
        char numStr[6] = {0};
        memcpy(numStr, data + 2, 5);
        dataOut.blockInfo.req.blockNumber = (uint32_t)strtoul(numStr, nullptr, 10);
        /* 文件系统: 第8字节为ASCII字符 */
        dataOut.blockInfo.req.fileSystem = data[7];
    }
    else
    {
        if (dataLen < 78)
        {
            return false;
        }
        dataOut.hasBlockInfo = true;
        dataOut.blockInfo.isRequest = isRequest;
        auto& resp = dataOut.blockInfo.resp;
        /* 第1-2字节: Block type(大端) */
        resp.blockType = ((uint16_t)data[0] << 8) | data[1];
        /* 第2-3字节: Length of Info(大端) */
        resp.lengthOfInfo = ((uint16_t)data[2] << 8) | data[3];
        /* 第4-5字节: Unknown blockinfo 2(大端) */
        resp.unknownBlockinfo2 = ((uint16_t)data[4] << 8) | data[5];
        /* 第6-7字节: Constant 3(大端) = "pp" = 0x7070 */
        resp.constant3 = ((uint16_t)data[6] << 8) | data[7];
        /* 第8字节: Unknown byte */
        resp.unknownByte1 = data[8];
        /* 第9字节: Block flags */
        resp.rawBlockFlags = data[9];
        resp.blockFlagsLinked = resp.rawBlockFlags & 0x01; /* bit0: .... ...1 */
        resp.blockFlagsStandardBlock = (resp.rawBlockFlags >> 1) & 0x01; /* bit1: .... ..1. */
        resp.blockFlagsNonRetain = (resp.rawBlockFlags >> 3) & 0x01; /* bit3: .... 1... */
        /* 第10字节: Block language */
        resp.blockLanguage = data[10];
        /* 第11字节: Subblk type */
        resp.subblkType = data[11];
        /* 第12-13字节: Block number(大端) */
        resp.blockNumber = ((uint16_t)data[12] << 8) | data[13];
        /* 第14-17字节: Length load memory(大端) */
        resp.lengthLoadMemory = ((uint32_t)data[14] << 24) | ((uint32_t)data[15] << 16) | ((uint32_t)data[16] << 8) | data[17];
        /* 第18-21字节: Block Security(4字节) */
        resp.blockSecurity = ((uint32_t)data[18] << 24) | ((uint32_t)data[19] << 16) | ((uint32_t)data[20] << 8) | data[21];
        /* 第22-27字节: Code timestamp(6字节) */
        memcpy(resp.codeTimestamp, data + 22, 6);
        /* 第28-33字节: Interface timestamp(6字节) */
        memcpy(resp.interfaceTimestamp, data + 28, 6);
        /* 第34-35字节: SSB length(大端) */
        resp.ssbLength = ((uint16_t)data[34] << 8) | data[35];
        /* 第36-37字节: ADD length(大端) */
        resp.addLength = ((uint16_t)data[36] << 8) | data[37];
        /* 第38-39字节: Localdata length(大端) */
        resp.localDataLength = ((uint16_t)data[38] << 8) | data[39];
        /* 第40-41字节: MC7 code length(大端) */
        resp.mc7CodeLength = ((uint16_t)data[40] << 8) | data[41];
        /* 第42-49字节: Author(8字节ASCII, 以00截止) */
        memcpy(resp.author, data + 42, 8);
        /* 第50-57字节: Family(8字节ASCII, 以00截止) */
        memcpy(resp.family, data + 50, 8);
        /* 第58-65字节: Name(8字节ASCII, 以00截止) */
        memcpy(resp.name, data + 58, 8);
        /* 第66字节: Version(高4位=主版本, 低4位=次版本) */
        auto version = data[66];
        resp.versionMajor = (version >> 4) & 0x0F;
        resp.versionMinor = version & 0x0F;
        /* 第67字节: Unknown byte */
        resp.unknownByte2 = data[67];
        /* 第68-69字节: Block checksum(大端) */
        resp.blockChecksum = ((uint16_t)data[68] << 8) | data[69];
        /* 第70-73字节: Reserved 1 */
        resp.reserved1 = ((uint32_t)data[70] << 24) | ((uint32_t)data[71] << 16) | ((uint32_t)data[72] << 8) | data[73];
        /* 第74-77字节: Reserved 2 */
        resp.reserved2 = ((uint32_t)data[74] << 24) | ((uint32_t)data[75] << 16) | ((uint32_t)data[76] << 8) | data[77];
    }
    return true;
}

bool S7CommParser::parseCpuServiceSzlData(const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceData& dataOut)
{
    if (dataLen < 4)
    {
        return false;
    }
    dataOut.hasSzl = true;
    /* 前4字节固定为 SZL-ID + SZL-Index */
    uint16_t szlId = ((uint16_t)data[0] << 8) | data[1];
    dataOut.szlHeader.szlId.rawId = szlId;
    dataOut.szlHeader.szlId.diagnosticType = (szlId >> 12) & 0x0F;
    dataOut.szlHeader.szlId.extractNumber = (szlId >> 8) & 0x0F;
    dataOut.szlHeader.szlId.partialListId = szlId & 0xFF;
    dataOut.szlHeader.szlIndex = ((uint16_t)data[2] << 8) | data[3];
    if (dataLen >= 8) /* 如果数据 >= 8 字节, 解析完整的SZL头部(响应格式) */
    {
        dataOut.szlHeader.listLength = ((uint16_t)data[4] << 8) | data[5];
        dataOut.szlHeader.listCount = ((uint16_t)data[6] << 8) | data[7];
        /* 解析SZL数据 */
        uint32_t offset = 8;
        for (uint16_t i = 0; i < dataOut.szlHeader.listCount; ++i)
        {
            if (offset + dataOut.szlHeader.listLength > dataLen)
            {
                break;
            }
            s7comm::SzlData d;
            d.length = dataOut.szlHeader.listLength;
            d.data = data + offset;
            dataOut.szlDatas.emplace_back(std::move(d));
            offset += dataOut.szlHeader.listLength;
        }
    }
    else /* 短数据(请求): 没有listLength/listCount/数据记录 */
    {
        dataOut.szlHeader.listLength = 0;
        dataOut.szlHeader.listCount = 0;
    }
    return true;
}

bool S7CommParser::parseCpuServiceMessageData(bool isRequest, const uint8_t* data, uint32_t dataLen, s7comm::CpuServiceData& dataOut)
{
    if (dataLen < 2)
    {
        return false;
    }
    dataOut.hasMessageService = true;
    dataOut.msgService.isRequest = isRequest;
    if (isRequest) /* 请求 */
    {
        /* 前2字节固定: 订阅事件掩码 + 保留字节 */
        uint8_t subscribedEvents = data[0];
        dataOut.msgService.req.modeTransition = subscribedEvents & 0x01; /* bit0: 模式切换 */
        dataOut.msgService.req.systemDiagnostics = (subscribedEvents >> 1) & 0x01; /* bit1: 系统诊断 */
        dataOut.msgService.req.userDefined = (subscribedEvents >> 2) & 0x01; /* bit2: 用户自定义 */
        dataOut.msgService.req.alarms = (subscribedEvents >> 7) & 0x01; /* bit7: 报警 */
        dataOut.msgService.req.reserved = data[1];
        /* 用户名: 从第3字节开始, 到数据末尾或遇到'\0'为止 */
        if (dataLen > 2)
        {
            const uint8_t* nameStart = data + 2;
            uint32_t nameLen = dataLen - 2;
            parseAsciiString(nameStart, nameLen, dataOut.msgService.req.username);
        }
    }
    else /* 响应 */
    {
        dataOut.msgService.resp.result = data[0];
        dataOut.msgService.resp.reserved = data[1];
    }
    return true;
}

bool S7CommParser::parseCpuServiceTimestamp(const uint8_t* data, uint32_t dataLen, s7comm::S7Timestamp& ts)
{
    if (dataLen < 10)
    {
        return false;
    }
    auto bcdDecode = [](uint8_t b) -> uint8_t { return ((b >> 4) & 0x0F) * 10 + (b & 0x0F); }; /* BCD解码辅助函数 */
    ts.reserved = data[0];
    ts.year1 = bcdDecode(data[1]);
    ts.year2 = bcdDecode(data[2]);
    ts.month = bcdDecode(data[3]);
    ts.day = bcdDecode(data[4]);
    ts.hour = bcdDecode(data[5]);
    ts.minute = bcdDecode(data[6]);
    ts.second = bcdDecode(data[7]);
    auto tenths = bcdDecode(data[8] >> 4); /* 高4位 */
    auto hundredths = bcdDecode(data[8] & 0x0F); /* 低4位 */
    auto thousandths = bcdDecode(data[9] >> 4); /* 高4位 */
    ts.milliseconds = tenths * 100 + hundredths * 10 + thousandths;
    ts.weekday = data[9] & 0x0F;
    return true;
}

bool S7CommParser::parseCpuServiceDataDetail(const s7comm::CpuServiceParam& param, s7comm::CpuServiceData& dataOut)
{
    if (param.items.empty() || 0xFF != dataOut.returnCode || 0 == dataOut.length || !dataOut.rawData) /* 非成功响应或无数据, 无需详细解析 */
    {
        return true;
    }
    const auto& item = param.items[0];
    uint32_t payloadLen = dataOut.length;
    const uint8_t* payload = dataOut.rawData;
    if (0x03 == item.functionGroup) /* Block functions(块功能) */
    {
        if (0x01 == item.subFunction)
        {
            if (0x09 == dataOut.transportSize && payloadLen >= 4) /* OCTET STRING */
            {
                return parseCpuServiceBlockListData(payload, payloadLen, dataOut);
            }
        }
        else if (0x02 == item.subFunction)
        {
            if (0x09 == dataOut.transportSize && payloadLen >= 2) /* OCTET STRING */
            {
                return parseCpuServiceBlockListOfTypeData(1 == item.type, payload, payloadLen, dataOut);
            }
        }
        else if (0x03 == item.subFunction)
        {
            if (0x09 == dataOut.transportSize && payloadLen >= 8) /* OCTET STRING */
            {
                return parseCpuServiceBlockInfoData(1 == item.type, payload, payloadLen, dataOut);
            }
        }
    }
    else if (0x04 == item.functionGroup) /* CPU functions(CPU功能) */
    {
        if (0x01 == item.subFunction)
        {
            return parseCpuServiceSzlData(payload, payloadLen, dataOut);
        }
        else if (0x02 == item.subFunction)
        {
            if (0x09 == dataOut.transportSize && payloadLen >= 2) /* OCTET STRING */
            {
                return parseCpuServiceMessageData(1 == item.type, payload, payloadLen, dataOut);
            }
        }
        else if (0x10 == item.subFunction || 0x11 == item.subFunction || 0x12 == item.subFunction || 0x13 == item.subFunction)
        {
            if (0x09 == dataOut.transportSize && payloadLen >= 10) /* OCTET STRING */
            {
                dataOut.hasTimestamp = true;
                return parseCpuServiceTimestamp(payload, payloadLen, dataOut.timestamp);
            }
        }
    }
    else if (0x07 == item.functionGroup) /* Time functions (时间功能) */
    {
        if (0x01 == item.subFunction || 0x02 == item.subFunction)
        {
            if (0x09 == dataOut.transportSize && payloadLen >= 10) /* OCTET STRING */
            {
                dataOut.hasTimestamp = true;
                return parseCpuServiceTimestamp(payload, payloadLen, dataOut.timestamp);
            }
        }
    }
    return true;
}

bool S7CommParser::parseCpuServiceData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                       s7comm::CpuServiceData& dataOut)
{
    if (dataLen < 1)
    {
        return false;
    }
    dataOut.returnCode = data[0];
    if (0xff != dataOut.returnCode || dataLen < 4)
    {
        return true;
    }
    /* 解析传输大小和长度 */
    dataOut.transportSize = data[1];
    dataOut.length = ((uint16_t)data[2] << 8) | data[3];
    dataOut.rawData = data + 4;
    return true;
}

bool S7CommParser::tryReassembleCpuServiceData(const std::chrono::steady_clock::time_point& ntp, const ProtocolHeader* header,
                                               s7comm::S7CommInfo& info)
{
    CpuServiceFragmentKey key;
    if (header)
    {
        if (header->parent)
        {
            if (NetworkProtocol::IPv4 == header->parent->getProtocol())
            {
                const Ipv4Header* ipHeader = (const Ipv4Header*)header->parent;
                memcpy(&key.srcIp, ipHeader->srcAddr, 4);
                memcpy(&key.dstIp, ipHeader->dstAddr, 4);
            }
        }
        if (TransportProtocol::TCP == header->getProtocol())
        {
            const TcpHeader* tcpHeader = (const TcpHeader*)header;
            key.srcPort = tcpHeader->srcPort;
            key.dstPort = tcpHeader->dstPort;
        }
    }
    key.dataUnitReferenceNumber = info.cpuParam.items[0].dataUnitReferenceNumber;
    const auto& item = info.cpuParam.items[0];
    if (0x00 == item.lastDataUnit) /* 最后一片 */
    {
        auto iter = m_cpuServiceFragments.find(key);
        if (m_cpuServiceFragments.end() != iter)
        {
            iter->second.lastAccess = ntp;
            if (info.cpuData.rawData && info.cpuData.length > 0) /* 合并之前缓存的数据 */
            {
                iter->second.data.insert(iter->second.data.end(), info.cpuData.rawData, info.cpuData.rawData + info.cpuData.length);
            }
            /* 追加长度: 只累加纯数据部分(去掉本片的4字节头部) */
            iter->second.totalDataLength += (info.header.dataLength >= 4 ? info.header.dataLength - 4 : 0);
            /* 更新头部: 第一片的PDU参考号, 总长度(仅含一个4字节头部) */
            info.header.protocolDataUnitReference = iter->second.firstProtocolDataUnitReference;
            info.header.dataLength = iter->second.totalDataLength;
            info.reassembledData = std::move(iter->second.data); /* 将合并后的数据转移到info中, 避免指针悬空 */
            m_cpuServiceFragments.erase(iter);
            if (info.reassembledData.empty())
            {
                info.cpuData.rawData = nullptr;
                info.cpuData.length = 0;
            }
            else
            {
                info.cpuData.rawData = info.reassembledData.data();
                info.cpuData.length = (uint16_t)(info.reassembledData.size());
            }
        }
        /* else: 没有缓存(单帧或缓存丢失), 直接使用当前帧数据 */
        return true; /* 重组完成 */
    }
    else if (0x01 == item.lastDataUnit) /* 非最后一片, 缓存数据 */
    {
        auto iter = m_cpuServiceFragments.find(key);
        if (m_cpuServiceFragments.end() == iter) /* 第一片: 创建新缓存并记录头部信息 */
        {
            S7FragmentCache frag;
            frag.lastAccess = ntp;
            frag.firstProtocolDataUnitReference = info.header.protocolDataUnitReference;
            frag.totalDataLength = info.header.dataLength;
            if (info.cpuData.rawData && info.cpuData.length > 0)
            {
                frag.data.insert(frag.data.end(), info.cpuData.rawData, info.cpuData.rawData + info.cpuData.length);
            }
            m_cpuServiceFragments[key] = std::move(frag);
        }
        else /* 中间片 */
        {
            iter->second.lastAccess = ntp;
            /* 追加长度: 只累加纯数据部分(去掉本片的4字节头部) */
            iter->second.totalDataLength += (info.header.dataLength >= 4 ? info.header.dataLength - 4 : 0);
            if (info.cpuData.rawData && info.cpuData.length > 0) /* 追加数据 */
            {
                iter->second.data.insert(iter->second.data.end(), info.cpuData.rawData, info.cpuData.rawData + info.cpuData.length);
            }
        }
        return false; /* 分片未完整 */
    }
    return true; /* 非分片情况, 直接解析 */
}

bool S7CommParser::parseReadWriteParamItem(const uint8_t* data, uint32_t dataLen, s7comm::ReadWriteParamItem& item, uint32_t& itemLen)
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
        item.length = ((uint16_t)data[4] << 8) | data[5];
        item.dbNumber = ((uint16_t)data[6] << 8) | data[7];
        item.area = data[8];
        item.address = ((uint32_t)data[9] << 16) | ((uint32_t)data[10] << 8) | (uint32_t)data[11];
        itemLen = 12;
        return true;
    }
    else if (0x82 == data[2] && dataLen >= 3) /* DBREAD格式: 0x82, 或优化访问格式等 */
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

bool S7CommParser::parseReadWriteParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                       s7comm::ReadWriteParam& param)
{
    if (dataLen < 2)
    {
        return false;
    }
    param.itemCount = data[1];
    if (0 == param.itemCount)
    {
        return true;
    }
    if (s7comm::RosctrType::JOB == rosctr)
    {
        uint32_t offset = 2;
        for (uint8_t i = 0; i < param.itemCount; ++i)
        {
            s7comm::ReadWriteParamItem item;
            uint32_t itemLen = 0;
            if (!parseReadWriteParamItem(data + offset, dataLen - offset, item, itemLen))
            {
                return false;
            }
            param.items.emplace_back(item);
            offset += itemLen;
        }
    }
    return true;
}

bool S7CommParser::parseReadWriteDataItem(const uint8_t* data, uint32_t dataLen, s7comm::ReadWriteData& item, uint32_t& itemLen)
{
    itemLen = 0;
    if (dataLen < 1)
    {
        return false;
    }
    uint32_t dataOffset = 0;
    item.returnCode = data[0]; /* 首字节为returnCode */
    dataOffset = 1;
    if (dataLen < 4)
    {
        itemLen = 1;
        return true;
    }
    /* 0x01-BIT, 0x02-BYTE, 0x03-BIT_ARRAY, 0x04-BYTE/WORD/DWORD, 0x05-INTEGER, 0x06-DWORD, 0x07-REAL, 0x09-OCTET STRING */
    item.transportSize = data[dataOffset];
    uint16_t lenRaw = ((uint16_t)data[dataOffset + 1] << 8) | data[dataOffset + 2];
    if (0x09 == item.transportSize) /* OCTET STRING是字节数 */
    {
        item.length = lenRaw;
    }
    else /* 其他类型(0x01/0x02/0x03/0x04/0x05/0x06/0x07等)都是bit数 */
    {
        item.length = (lenRaw + 7) / 8;
    }
    uint32_t totalLen = 4 + item.length;
    /* 如果数据长度为奇数, 且实际数据长度足够, 则包含填充字节(西门子会填充1字节使其为偶数) */
    if (0 != item.length % 2 && dataLen >= totalLen + 1)
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

bool S7CommParser::parseReadWriteData(const s7comm::RosctrType& rosctr, const s7comm::FunctionCode& funcCode, const uint8_t* data,
                                      uint32_t dataLen, uint8_t itemCount, std::vector<s7comm::ReadWriteData>& dataOut)
{
    if (s7comm::RosctrType::ACK_DATA == rosctr && s7comm::FunctionCode::READ_VARIABLE == funcCode)
    {
        /* 读响应: 数据项列表, 每项含returnCode + transportSize + length + data */
        uint32_t offset = 0;
        for (uint8_t i = 0; i < itemCount; ++i)
        {
            s7comm::ReadWriteData item;
            uint32_t itemLen = 0;
            if (!parseReadWriteDataItem(data + offset, dataLen - offset, item, itemLen))
            {
                return false;
            }
            dataOut.emplace_back(item);
            offset += itemLen;
        }
    }
    else if (s7comm::RosctrType::JOB == rosctr && s7comm::FunctionCode::WRITE_VARIABLE == funcCode)
    {
        /* 写请求: 数据项列表, 每项含transportSize + length + data(无returnCode) */
        uint32_t offset = 0;
        for (uint8_t i = 0; i < itemCount; ++i)
        {
            s7comm::ReadWriteData item;
            uint32_t itemLen = 0;
            if (!parseReadWriteDataItem(data + offset, dataLen - offset, item, itemLen))
            {
                return false;
            }
            dataOut.emplace_back(item);
            offset += itemLen;
        }
    }
    else if (s7comm::RosctrType::ACK_DATA == rosctr && s7comm::FunctionCode::WRITE_VARIABLE == funcCode)
    {
        /* 写响应: 仅包含每个item的returnCode(1字节), 无transportSize/length/data */
        if (dataLen < itemCount)
        {
            return false;
        }
        for (uint8_t i = 0; i < itemCount; ++i)
        {
            s7comm::ReadWriteData item;
            item.returnCode = data[i];
            item.transportSize = 0;
            item.length = 0;
            item.data = nullptr;
            dataOut.emplace_back(item);
        }
    }
    return true;
}

bool S7CommParser::parseRequestDownloadParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                             s7comm::RequestDownloadParam& param)
{
    if (dataLen < 1)
    {
        return false;
    }
    if (s7comm::RosctrType::JOB == rosctr)
    {
        if (dataLen < 9) /* 最小长度检查, 确保能安全访问到filenameLen字段(offset=8) */
        {
            return true; /* 数据不足, 只解析functionCode */
        }
        /* 字节1: 功能状态(位域) */
        param.funcStatus.moreDataFollowing = (data[1] & 0x01) ? 1 : 0;
        param.funcStatus.error = (data[1] & 0x02) ? 1 : 0;
        /* 字节2-3: 保留控制字1(大端) */
        param.unknownByteInBlockControl1 = ((uint16_t)data[2] << 8) | data[3];
        /* 字节4-7: 保留控制字2(大端) */
        param.unknownByteInBlockControl2 =
            ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[7];
        /* 字节8: 文件名长度(通常为9) */
        param.filenameLen = data[8];
        /* 字节9-17: 文件名(固定9字节结构) */
        uint32_t offset = 9;
        if (offset + param.filenameLen > dataLen)
        {
            return false;
        }
        /* 按实际文件名长度安全读取, 避免filenameLen<<9时越界, 同时清零结构体防止残留 */
        uint8_t copyLen = (param.filenameLen < 9) ? param.filenameLen : 9;
        memset(&param.filename, 0, sizeof(param.filename));
        if (copyLen >= 1)
        {
            param.filename.fileIdentifier = (char)data[offset];
        }
        if (copyLen >= 2)
        {
            param.filename.blockType[0] = (char)data[offset + 1];
        }
        if (copyLen >= 3)
        {
            param.filename.blockType[1] = (char)data[offset + 2];
        }
        if (copyLen >= 4)
        {
            param.filename.blockNumber[0] = (char)data[offset + 3];
        }
        if (copyLen >= 5)
        {
            param.filename.blockNumber[1] = (char)data[offset + 4];
        }
        if (copyLen >= 6)
        {
            param.filename.blockNumber[2] = (char)data[offset + 5];
        }
        if (copyLen >= 7)
        {
            param.filename.blockNumber[3] = (char)data[offset + 6];
        }
        if (copyLen >= 8)
        {
            param.filename.blockNumber[4] = (char)data[offset + 7];
        }
        if (copyLen >= 9)
        {
            param.filename.destFileSystem = (char)data[offset + 8];
        }
        offset += param.filenameLen; /* 按协议声明的长度跳过, 而非固定9 */
        /* 字节18+: 可选扩展字段 */
        if (offset >= dataLen)
        {
            return true;
        }
        param.lengthPart2 = data[offset];
        offset++;
        if (offset >= dataLen)
        {
            return true;
        }
        param.unknownCharBeforeLoadMem = (char)data[offset];
        offset++;
        /* 装载内存大小(6字节ASCII数字) */
        if (offset + 6 > dataLen)
        {
            return true;
        }
        for (uint8_t i = 0; i < 6; ++i)
        {
            param.lengthOfLoadMemory[i] = (char)data[offset + i];
        }
        offset += 6;
        /* MC7代码大小(6字节ASCII数字) */
        if (offset + 6 > dataLen)
        {
            return true;
        }
        for (uint8_t i = 0; i < 6; ++i)
        {
            param.lengthOfMc7Code[i] = (char)data[offset + i];
        }
    }
    else if (s7comm::RosctrType::ACK_DATA == rosctr)
    {
        if (dataLen >= 2) /* 如果 dataLen > 1, 可能包含funcStatus */
        {
            param.funcStatus.moreDataFollowing = (data[1] & 0x01) ? 1 : 0;
            param.funcStatus.error = (data[1] & 0x02) ? 1 : 0;
        }
    }
    return true;
}

bool S7CommParser::parseRequestDownloadData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                            s7comm::RequestDownloadData& dataOut)
{
    /* 空数据 */
    return true;
}

bool S7CommParser::parseDownloadBlockParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                           s7comm::DownloadBlockParam& param)
{
    if (dataLen < 1)
    {
        return false;
    }
    if (s7comm::RosctrType::JOB == rosctr)
    {
        if (dataLen < 9) /* JOB分支最小长度检查: 至少9字节(到filenameLen) */
        {
            return false;
        }
        /* 字节1: 功能状态(位域) */
        param.funcStatus.moreDataFollowing = (data[1] & 0x01) ? 1 : 0;
        param.funcStatus.error = (data[1] & 0x02) ? 1 : 0;
        /* 字节2-3: 保留控制字1(大端) */
        param.unknownByteInBlockControl1 = ((uint16_t)data[2] << 8) | data[3];
        /* 字节4-7: 保留控制字2(大端) */
        param.unknownByteInBlockControl2 =
            ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[7];
        /* 字节8: 文件名长度(通常为9) */
        param.filenameLen = data[8];
        /* 字节9-17: 文件名(固定9字节结构) */
        uint32_t offset = 9;
        if (offset + param.filenameLen > dataLen)
        {
            return false;
        }
        /* 按实际文件名长度安全读取, 避免越界 */
        uint8_t copyLen = (param.filenameLen < 9) ? param.filenameLen : 9;
        memset(&param.filename, 0, sizeof(param.filename));
        if (copyLen >= 1)
        {
            param.filename.fileIdentifier = (char)data[offset];
        }
        if (copyLen >= 2)
        {
            param.filename.blockType[0] = (char)data[offset + 1];
        }
        if (copyLen >= 3)
        {
            param.filename.blockType[1] = (char)data[offset + 2];
        }
        if (copyLen >= 4)
        {
            param.filename.blockNumber[0] = (char)data[offset + 3];
        }
        if (copyLen >= 5)
        {
            param.filename.blockNumber[1] = (char)data[offset + 4];
        }
        if (copyLen >= 6)
        {
            param.filename.blockNumber[2] = (char)data[offset + 5];
        }
        if (copyLen >= 7)
        {
            param.filename.blockNumber[3] = (char)data[offset + 6];
        }
        if (copyLen >= 8)
        {
            param.filename.blockNumber[4] = (char)data[offset + 7];
        }
        if (copyLen >= 9)
        {
            param.filename.destFileSystem = (char)data[offset + 8];
        }
    }
    else if (s7comm::RosctrType::ACK_DATA == rosctr)
    {
        if (dataLen < 2) /* ACK_DATA分支增加最小长度检查, 防止data[1]越界 */
        {
            return false;
        }
        /* 字节1: 功能状态(位域) */
        param.funcStatus.moreDataFollowing = (data[1] & 0x01) ? 1 : 0;
        param.funcStatus.error = (data[1] & 0x02) ? 1 : 0;
    }
    return true;
}

bool S7CommParser::parseDownloadBlockData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                          s7comm::DownloadBlockData& dataOut)
{
    if (dataLen < 4)
    {
        return false;
    }
    dataOut.length = ((uint16_t)data[0] << 8) | data[1]; /* 字节0: 长度 */
    dataOut.unknownByteInBlockControl = ((uint16_t)data[2] << 8) | data[3]; /* 字节1-2: 保留控制字 */
    if (dataOut.length > 0 && dataLen >= 4 + dataOut.length)
    {
        dataOut.data = data + 4;
    }
    return true;
}

bool S7CommParser::parseDownloadEndedParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                           s7comm::DownloadEndedParam& param)
{
    if (dataLen < 1) /* 入口检查改为1, ACK_DATA分支仅需functionCode; JOB分支单独检查 */
    {
        return false;
    }
    if (s7comm::RosctrType::JOB == rosctr)
    {
        if (dataLen < 9) /* JOB分支最小长度检查: 至少9字节(到filenameLen字段) */
        {
            return false;
        }
        /* 字节1: 功能状态(位域) */
        param.funcStatus.moreDataFollowing = (data[1] & 0x01) ? 1 : 0;
        param.funcStatus.error = (data[1] & 0x02) ? 1 : 0;
        /* 字节2-3: 错误码 */
        param.errorCode = ((uint16_t)data[2] << 8) | data[3];
        /* 字节4-7: 保留控制字 */
        param.unknownByteInBlockControl =
            ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[7];
        /* 字节8: 文件名长度(通常为9) */
        param.filenameLen = data[8];
        /* 字节9-17: 文件名(固定9字节结构) */
        uint32_t offset = 9;
        if (offset + param.filenameLen > dataLen)
        {
            return false;
        }
        /* 按实际文件名长度安全读取, 避免越界 */
        uint8_t copyLen = (param.filenameLen < 9) ? param.filenameLen : 9;
        memset(&param.filename, 0, sizeof(param.filename));
        if (copyLen >= 1)
        {
            param.filename.fileIdentifier = (char)data[offset];
        }
        if (copyLen >= 2)
        {
            param.filename.blockType[0] = (char)data[offset + 1];
        }
        if (copyLen >= 3)
        {
            param.filename.blockType[1] = (char)data[offset + 2];
        }
        if (copyLen >= 4)
        {
            param.filename.blockNumber[0] = (char)data[offset + 3];
        }
        if (copyLen >= 5)
        {
            param.filename.blockNumber[1] = (char)data[offset + 4];
        }
        if (copyLen >= 6)
        {
            param.filename.blockNumber[2] = (char)data[offset + 5];
        }
        if (copyLen >= 7)
        {
            param.filename.blockNumber[3] = (char)data[offset + 6];
        }
        if (copyLen >= 8)
        {
            param.filename.blockNumber[4] = (char)data[offset + 7];
        }
        if (copyLen >= 9)
        {
            param.filename.destFileSystem = (char)data[offset + 8];
        }
    }
    else if (s7comm::RosctrType::ACK_DATA == rosctr)
    {
        /* 空数据 */
    }
    return true;
}

bool S7CommParser::parseDownloadEndedData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                          s7comm::DownloadEndedData& dataOut)
{
    /* 空数据 */
    return true;
}

bool S7CommParser::parseStartUploadParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                         s7comm::StartUploadParam& param)
{
    if (dataLen < 4)
    {
        return false;
    }
    param.blockTypeLen = data[1];
    param.blockNumLen = data[2];
    param.fileSystemLen = data[3];
    uint32_t offset = 4;
    if (2 == param.blockTypeLen && offset + 2 <= dataLen)
    {
        memcpy(param.blockType, data + offset, 2);
        offset += 2;
    }
    if (param.blockNumLen <= 5 && offset + param.blockNumLen <= dataLen)
    {
        memcpy(param.blockNumber, data + offset, param.blockNumLen);
        offset += param.blockNumLen;
    }
    if (1 == param.fileSystemLen && offset < dataLen)
    {
        param.fileSystem = data[offset];
    }
    return true;
}

bool S7CommParser::parseStartUploadData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                        s7comm::StartUploadData& dataOut)
{
    if (dataLen < 5)
    {
        return false;
    }
    dataOut.uploadId = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | (uint32_t)data[3];
    dataOut.status = data[4];
    return true;
}

bool S7CommParser::parseUploadParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::UploadParam& param)
{
    if (dataLen < 5)
    {
        return false;
    }
    param.uploadId = ((uint32_t)data[1] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 8) | (uint32_t)data[4];
    return true;
}

bool S7CommParser::parseUploadData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::UploadData& dataOut)
{
    if (dataLen < 5)
    {
        return false;
    }
    dataOut.dataOffset = (data[0] << 8) | data[1];
    dataOut.dataLen = (data[2] << 8) | data[3];
    dataOut.isLastBlock = (data[4] == 0x01);
    if (dataOut.dataLen > 0 && dataLen >= 5 + dataOut.dataLen)
    {
        dataOut.blockData = data + 5;
    }
    return true;
}

bool S7CommParser::parseEndUploadParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                       s7comm::EndUploadParam& param)
{
    if (dataLen < 5)
    {
        return false;
    }
    param.uploadId = ((uint32_t)data[1] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 8) | (uint32_t)data[4];
    return true;
}

bool S7CommParser::parseEndUploadData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                      s7comm::EndUploadData& dataOut)
{
    if (dataLen < 1)
    {
        return false;
    }
    dataOut.returnCode = data[0];
    if (dataLen >= 3)
    {
        dataOut.errorClass = data[1];
        dataOut.errorCode = data[2];
    }
    return true;
}

bool S7CommParser::parsePlcControlParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                        s7comm::PlcControlParam& param)
{
    if (dataLen < 2)
    {
        return false;
    }
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

bool S7CommParser::parsePlcControlData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                       s7comm::PlcControlData& dataOut)
{
    if (dataLen < 1)
    {
        return false;
    }
    dataOut.returnCode = data[0];
    if (dataLen > 1)
    {
        dataOut.statusMsg = std::string((const char*)data + 1, dataLen - 1);
    }
    return true;
}

bool S7CommParser::parsePiServiceParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                       s7comm::PiServiceParam& param)
{
    if (dataLen < 1)
    {
        return false;
    }
    if (dataLen < 10)
    {
        return true;
    }
    param.unknownBytes = ((uint64_t)data[1] << 48) | ((uint64_t)data[2] << 40) | ((uint64_t)data[3] << 32) | ((uint64_t)data[4] << 24)
                         | ((uint64_t)data[5] << 16) | ((uint64_t)data[6] << 8) | data[7];
    /* 参数块长度 */
    param.paramBlockLength = ((uint16_t)data[8] << 8) | data[9];
    /* 解析参数块 */
    uint32_t offset = 10;
    if (param.paramBlockLength > 0 && offset + param.paramBlockLength <= dataLen)
    {
        /* 块数量 */
        if (offset < dataLen)
        {
            param.paramBlock.numberOfBlocks = data[offset];
            offset += 1;
        }
        /* 未知字节 */
        if (offset < dataLen)
        {
            param.paramBlock.unknownByte = data[offset];
            offset += 1;
        }
        /* 文件名(8字节ASCII) */
        if (offset + 8 <= dataLen)
        {
            memcpy(param.paramBlock.filename.blockType, data + offset, 2);
            offset += 2;
            memcpy(param.paramBlock.filename.blockNumber, data + offset, 5);
            offset += 5;
            param.paramBlock.filename.destFileSystem = data[offset];
            offset += 1;
        }
    }
    /* PI服务名 */
    if (offset < dataLen)
    {
        param.serviceNameLength = data[offset];
        offset += 1;
        if (param.serviceNameLength > 0 && offset + param.serviceNameLength <= dataLen)
        {
            uint8_t copyLen = param.serviceNameLength < 31 ? param.serviceNameLength : 31;
            memcpy(param.serviceName, data + offset, copyLen);
        }
    }
    return true;
}

bool S7CommParser::parsePiServiceData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                      s7comm::PiServiceData& dataOut)
{
    if (dataLen < 1)
    {
        return false;
    }
    dataOut.returnCode = data[0];
    dataOut.dataLen = dataLen - 1;
    if (dataOut.dataLen > 0)
    {
        dataOut.responseData = data + 1;
    }
    return true;
}

bool S7CommParser::parsePlcStopParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::PlcStopParam& param)
{
    if (dataLen < 1)
    {
        return false;
    }
    return true;
}

bool S7CommParser::parsePlcStopData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::PlcStopData& dataOut)
{
    if (dataLen < 2)
    {
        return false;
    }
    dataOut.returnCode = data[0];
    dataOut.stopStatus = data[1];
    return true;
}

bool S7CommParser::parseCopyRamToRomParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                          s7comm::CopyRamToRomParam& param)
{
    if (dataLen < 2)
    {
        return false;
    }
    param.paramCount = data[1];
    uint32_t offset = 2;
    if (param.paramCount > 0 && offset + 8 <= dataLen)
    {
        memcpy(param.blockType, data + offset, 2);
        memcpy(param.blockNumber, data + offset + 2, 5);
    }
    return true;
}

bool S7CommParser::parseCopyRamToRomData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                         s7comm::CopyRamToRomData& dataOut)
{
    if (dataLen < 2)
    {
        return false;
    }
    dataOut.returnCode = data[0];
    dataOut.copyStatus = data[1];
    return true;
}

bool S7CommParser::parseCompressParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::CompressParam& param)
{
    if (dataLen < 3)
    {
        return false;
    }
    param.memoryType = data[1];
    param.reserved = data[2];
    return true;
}

bool S7CommParser::parseCompressData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen, s7comm::CompressData& dataOut)
{
    if (dataLen < 6)
    {
        return false;
    }
    dataOut.returnCode = data[0];
    dataOut.compressStatus = data[1];
    dataOut.freedBytes = ((uint32_t)data[2] << 24) | ((uint32_t)data[3] << 16) | ((uint32_t)data[4] << 8) | (uint32_t)data[5];
    return true;
}

bool S7CommParser::parseBlockOperationParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                            s7comm::BlockOperationParam& param)
{
    if (dataLen < 2)
    {
        return false;
    }
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
            memcpy(param.blockType, data + offset, 2);
            memcpy(param.blockNumber, data + offset + 2, 5);
            param.fileSystem = data[offset + 7];
        }
        offset += paramLen;
    }
    return true;
}

bool S7CommParser::parseBlockOperationData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                           s7comm::BlockOperationData& dataOut)
{
    if (dataLen < 1)
    {
        return false;
    }
    dataOut.returnCode = data[0];
    if (dataLen >= 2)
    {
        dataOut.blockStatus = data[1];
    }
    if (dataLen >= 4)
    {
        dataOut.errorClass = data[2];
        dataOut.errorCode = data[3];
    }
    return true;
}

bool S7CommParser::parseSetupCommParam(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                       s7comm::SetupCommParam& param)
{
    if (dataLen < 8)
    {
        return false;
    }
    param.reserved = data[1];
    param.maxAmqCalling = ((uint16_t)data[2] << 8) | data[3];
    param.maxAmqCalled = ((uint16_t)data[4] << 8) | data[5];
    param.pduLength = ((uint16_t)data[6] << 8) | data[7];
    return true;
}

bool S7CommParser::parseSetupCommData(const s7comm::RosctrType& rosctr, const uint8_t* data, uint32_t dataLen,
                                      s7comm::SetupCommData& dataOut)
{
    if (dataLen < 8)
    {
        return false;
    }
    dataOut.returnCode = data[0];
    dataOut.maxAmqCaller = ((uint16_t)data[2] << 8) | data[3];
    dataOut.maxAmqCallee = ((uint16_t)data[4] << 8) | data[5];
    dataOut.pduLength = ((uint16_t)data[6] << 8) | data[7];
    return true;
}
} // namespace npacket
