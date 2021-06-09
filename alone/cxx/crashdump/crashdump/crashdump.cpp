#include "crashdump.h"

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

/*********************************************************************
 业务逻辑
*********************************************************************/
std::string g_procFile; /* 当前程序文件全路径名 */
std::string g_outputPath; /* 崩溃堆栈文件输出路径 */
DumpCallback g_callback = nullptr; /* 崩溃回调 */
google_breakpad::ExceptionHandler* g_execptionHandler = nullptr; /* 异常句柄 */

/**
 * @brief 崩溃处理回调
 */
bool dumpHandler(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
{
    std::string cmd = "dump_assist.sh -pname " + g_procFile + " -dname " + descriptor.path();
    auto results = shellCmd(cmd);
    /* 回调到外部 */
    if (g_callback)
    {
        std::string json;
        if (!results.empty())
        {
            json = results[results.size() - 1];
        }
        g_callback(json);
    }
    return succeeded;
}

void open(const std::string& procFile, const std::string& outputPath, const DumpCallback& callback)
{
    static bool s_opened = false;
    if (s_opened)
    {
        return;
    }
    s_opened = true;
    /* 参数保存 */
    g_procFile = procFile;
    g_outputPath = outputPath;
    g_callback = callback;
    /* 创建路径 */
    createPath(g_outputPath);
    /* 开始监听 */
    google_breakpad::MinidumpDescriptor descriptor(g_outputPath.c_str());
    g_execptionHandler = new google_breakpad::ExceptionHandler(descriptor, NULL, dumpHandler, NULL, true, -1);
}
} // namespace crashdump
