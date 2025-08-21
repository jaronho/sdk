#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace nsocket
{
namespace ftp
{
/**
 * @brief 命令
 */
enum class Command
{
    USER,
    PASS,
    QUIT,
    PORT,
    PASV,
    TYPE,
    RETR,
    STOR,
    LIST,
    NLST,
    CWD,
    CDUP,
    PWD,
    MKD,
    RMD,
    DELE,
    RNFR,
    RNTO,
    SYST,
    NOOP,
    UNKNOWN
};

/**
 * @brief 响应码
 */
enum class ReplyCode
{
    Success = 200,
    Ready = 220,
    Closing = 221,
    DataConnOpen = 225,
    TransferComplete = 226,
    PassiveMode = 227,
    LoginSuccess = 230,
    NeedPassword = 331,
    NeedAccount = 332,
    FileActionPending = 350,
    CommandNotImplemented = 502,
    BadCommand = 500,
    BadArguments = 501,
    NotLoggedIn = 530,
    FileUnavailable = 550
};

/**
 * @brief 响应码
 */
enum class TransferType
{
    ASCII,
    BINARY
};

// 解析FTP命令
Command parseCommand(const std::string& cmd);
} // namespace ftp
} // namespace nsocket
