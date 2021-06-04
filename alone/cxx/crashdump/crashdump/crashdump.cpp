#include "crashdump.h"

#include <iostream>
#include <string.h>
#include <sys/timeb.h>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "client/linux/handler/exception_handler.h"
#endif

namespace crashdump
{
/*********************************************************************
 基础接口
*********************************************************************/

/**
 * @brief 创建路径
 * @param path 路径
 * @return true-成功, false-失败
 */
bool createPath(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }
    std::string pathNode;
    for (size_t i = 0, pathLen = path.length(); i < pathLen; ++i)
    {
        char pathChar = path.at(i);
        bool newPath = false;
        if ('/' == pathChar || '\\' == pathChar)
        {
#ifdef _WIN32
            pathChar = '\\';
#else
            pathChar = '/';
#endif
            newPath = true;
        }
        else if (pathLen - 1 == i)
        {
            newPath = true;
        }
        pathNode += pathChar;
        if (newPath)
        {
#ifdef _WIN32
            if (0 != _access(pathNode.c_str(), 0))
            {
                if (0 != _mkdir(pathNode.c_str()))
                {
                    return false;
                }
            }
#else
            if (0 != access(pathNode.c_str(), F_OK))
            {
                if (0 != mkdir(pathNode.c_str(), S_IRWXU | S_IRWXG | S_IRWXO))
                {
                    return false;
                }
            }
#endif
        }
    }
    return true;
}

/**
 * @brief 执行命令
 * @param cmd 命令
 * @return 执行结果
 */
std::vector<std::string> shellCmd(const std::string& cmd)
{
    std::vector<std::string> results;
    if (cmd.empty())
    {
        return results;
    }
    FILE* stream = nullptr;
#ifdef _WIN32
    stream = _popen(cmd.c_str(), "r");
#else
    stream = popen(cmd.c_str(), "r");
#endif
    if (!stream)
    {
        return results;
    }
    char line[1024] = {0};
    while (memset(line, 0, sizeof(line)) && fgets(line, sizeof(line) - 1, stream))
    {
        line[strlen(line) - 1] = '\0';
        if (strlen(line) > 0)
        {
            results.push_back(line);
        }
    }
#ifdef _WIN32
    _pclose(stream);
#else
    pclose(stream);
#endif
    return results;
}

/**
 * @brief 字符串分割
 * @param str 字符串
 * @param pattern 分割符
 * @return 字符串数组
 */
std::vector<std::string> splitString(std::string str, const std::string& pattern)
{
    std::vector<std::string> result;
    if (str.empty() || pattern.empty())
    {
        return result;
    }
    str.append(pattern);
    std::string::size_type pos;
    for (size_t i = 0; i < str.size(); ++i)
    {
        pos = str.find(pattern, i);
        if (pos < str.size())
        {
            result.push_back(str.substr(i, pos - i));
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}

/**
 * @brief 解析文件名信息
 * @param filePath 文件路径
 * @return 文件名信息数组, [0]-路径, [1]-文件名, [2]-基础名, [3]-后缀名
 */
std::vector<std::string> stripFileInfo(const std::string& filePath)
{
    std::string dirname = "", filename = filePath, basename = "", extname = "";
    size_t pos = filePath.find_last_of("/\\");
    if (pos < filePath.size())
    {
        dirname = filePath.substr(0, pos + 1);
        filename = filePath.substr(pos + 1, filePath.size() - 1);
    }
    pos = filename.find_last_of(".");
    if (pos < filename.size())
    {
        basename = filename.substr(0, pos);
        extname = filename.substr(pos, filename.size() - 1);
    }
    else
    {
        basename = filename;
    }
    std::vector<std::string> infos;
    infos.push_back(dirname);
    infos.push_back(filename);
    infos.push_back(basename);
    infos.push_back(extname);
    return infos;
}

/**
 * @brief 修正路径(末尾会带上路径符)
 * @param path 路径
 * @return 新路径
 */
std::string revisalPath(std::string path)
{
    if (path.empty())
    {
        return path;
    }
#ifdef _WIN32
    if (!path.empty() && '\\' != path.at(path.size() - 1))
    {
        path.append("\\");
    }
#else
    if (!path.empty() && '/' != path.at(path.size() - 1))
    {
        path.append("/");
    }
#endif
    return path;
}

/**
 * @brief 重命名文件
 * @param oldFilename 旧的文件名
 * @param newFilename 新的文件名
 * @param forceRename 是否强制命名(新的文件名已有文件存在时, 是否强制删除)
 * @return true-成功, false-失败
 */
bool renameFile(const std::string& oldFilename, const std::string& newFilename, bool forceRename)
{
    if (oldFilename.empty() || newFilename.empty())
    {
        return false;
    }
    if (forceRename)
    {
        remove(newFilename.c_str());
    }
    return 0 == rename(oldFilename.c_str(), newFilename.c_str());
}

/*********************************************************************
 业务逻辑
*********************************************************************/
std::string g_outputPath; /* 文件输出路径 */
std::string g_fullProcName; /* 当前程序名(包含绝对路径) */
std::string g_procFilename; /* 当前程序文件名 */
std::string g_procBasename; /* 当前程序文件基础名 */
std::string g_symbolPath; /* 符号文件路径 */
std::string g_symbolFilename; /* 符号文件名 */
DumpCallback g_dumpCallback = nullptr; /* 崩溃回调 */
google_breakpad::ExceptionHandler* g_execptionHandler = nullptr; /* 异常句柄 */

/**
 * @brief 生成符号文件
 */
bool generateSymbolFile()
{
    std::string symbolFilePath = revisalPath(g_symbolPath + g_procFilename);
    if (!createPath(symbolFilePath))
    {
        return false;
    }
    std::string tmpFullSymbolName = symbolFilePath + g_symbolFilename;
    /* 创建文件 */
    std::string cmd = "dump_syms " + g_fullProcName + " > " + tmpFullSymbolName;
    std::vector<std::string> result = shellCmd(cmd);
    if (!result.empty())
    {
        return false;
    }
    /* 创建目录 */
    cmd = "head -n1 " + tmpFullSymbolName;
    result = shellCmd(cmd);
    if (1 != result.size())
    {
        return false;
    }
    result = splitString(result[0], " "); /* 使用空格分割信息 */
    if (result.size() < 2 || 0 != result[result.size() - 1].compare(g_procFilename)) /* 最后一项必须是程序名称 */
    {
        return false;
    }
    std::string checksum = result[result.size() - 2]; /* 解析符号文件校验码 */
    symbolFilePath = revisalPath(symbolFilePath + checksum); /* 替换真正的符号文件路径 */
    if (!createPath(symbolFilePath))
    {
        return false;
    }
    /* 移动文件 */
    cmd = "mv " + tmpFullSymbolName + " " + symbolFilePath;
    result = shellCmd(cmd);
    if (!result.empty())
    {
        return false;
    }
    return true;
}

/**
 * @brief 崩溃处理回调
 */
bool dumpHandler(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
{
    /* 根据时间生成文件基础名称 */
    time_t now;
    time(&now);
    struct tm t = *localtime(&now);
    char datetime[16] = {0};
    strftime(datetime, sizeof(datetime), "%Y%m%d%H%M%S", &t);
    struct timeb tb;
    ftime(&tb);
    char ms[4] = {0};
    sprintf(ms, "%03d", tb.millitm);
    std::string newBaseName = g_procBasename + "_" + datetime + ms;
    /* 重命名dump文件 */
    std::string fullDumpName = descriptor.path();
    std::string newFullDumpName = g_outputPath + newBaseName + ".dmp";
    if (renameFile(fullDumpName, newFullDumpName, true))
    {
        fullDumpName = newFullDumpName;
    }
    /* 生成符号文件 */
    if (generateSymbolFile())
    {
        std::string translateDumpName = g_outputPath + newBaseName + ".txt";
        /* 翻译dump文件 */
        std::string cmd = "minidump_stackwalk " + fullDumpName + " " + g_symbolPath + " > " + translateDumpName;
        if (shellCmd(cmd).empty())
        {
            cmd = "rm " + fullDumpName;
            shellCmd(cmd);
            fullDumpName = translateDumpName;
        }
    }
    /* 回调到外部 */
    if (g_dumpCallback)
    {
        g_dumpCallback(fullDumpName);
    }
    return succeeded;
}

void open(const std::string& outputPath, const std::string& fullProcName, const DumpCallback& callback)
{
    static bool s_opened = false;
    if (s_opened)
    {
        return;
    }
    s_opened = true;
    /* 参数保存和解析 */
    g_outputPath = revisalPath(outputPath);
    g_fullProcName = fullProcName;
    std::vector<std::string> procFileInfo = stripFileInfo(fullProcName);
    g_procFilename = procFileInfo[1];
    g_procBasename = procFileInfo[2];
    g_symbolPath = revisalPath(g_outputPath + "symbols");
    g_symbolFilename = g_procBasename + ".sym";
    g_dumpCallback = callback;
    /* 创建路径 */
    createPath(g_outputPath);
    createPath(g_symbolPath);
    /* 开始监听 */
    google_breakpad::MinidumpDescriptor descriptor(g_outputPath.c_str());
    g_execptionHandler = new google_breakpad::ExceptionHandler(descriptor, NULL, dumpHandler, NULL, true, -1);
}
} // namespace crashdump
