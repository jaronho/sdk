#include "crashdump.h"

#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
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
    std::string parentPath;
    for (size_t i = 0, pathLen = path.size(); i < pathLen; ++i)
    {
        const char& ch = path[i];
        parentPath.push_back(ch);
        if ('/' == ch || '\\' == ch || pathLen - 1 == i)
        {
#ifdef _WIN32
            if (0 != _access(parentPath.c_str(), 0))
            {
                if (0 != _mkdir(parentPath.c_str()))
                {
                    return false;
                }
            }
#else
            if (0 != access(parentPath.c_str(), F_OK))
            {
                if (0 != mkdir(parentPath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO))
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
 * @brief 解析文件名信息
 * @param fullName 文件路径
 * @return 文件名信息数组, [0]-路径, [1]-文件名, [2]-基础名, [3]-后缀名(不包含.)
 */
std::vector<std::string> stripFileInfo(const std::string& fullName)
{
    std::string path = "", filename = fullName, basename = "", extname = "";
    size_t pos = fullName.find_last_of("/\\");
    if (pos < fullName.size())
    {
        path = fullName.substr(0, pos + 1);
        filename = fullName.substr(pos + 1, fullName.size() - 1);
    }
    pos = filename.find_last_of(".");
    if (pos < filename.size())
    {
        basename = filename.substr(0, pos);
        extname = filename.substr(pos + 1, filename.size() - 1);
    }
    else
    {
        basename = filename;
    }
    std::vector<std::string> infos;
    infos.emplace_back(path);
    infos.emplace_back(filename);
    infos.emplace_back(basename);
    infos.emplace_back(extname);
    return infos;
}

/**
 * @brief 执行命令
 * @param cmd 命令
 * @return 执行结果
 */
int shellCmd(const std::string& cmd, std::vector<std::string>* result = nullptr)
{
    if (cmd.empty())
    {
        return -1;
    }
    FILE* stream = NULL;
#ifdef _WIN32
    stream = _popen(cmd.c_str(), "r");
#else
    stream = popen(cmd.c_str(), "r");
#endif
    if (!stream)
    {
        return -2;
    }
    if (result)
    {
        (*result).clear();
        const size_t bufferSize = 1024;
        char buffer[bufferSize] = {0};
        std::string line;
        while (memset(buffer, 0, bufferSize) && fgets(buffer, bufferSize - 1, stream))
        {
            line += buffer;
            size_t pos = line.find('\n');
            if (std::string::npos != pos)
            {
                (*result).emplace_back(line.substr(0, pos));
                line = line.substr(pos + 1, line.size() - pos);
            }
        }
    }
#ifdef _WIN32
    return _pclose(stream);
#else
    return pclose(stream);
#endif
}

#if _WIN32
static std::string wstring2string(const std::wstring& wstr)
{
    if (wstr.empty())
    {
        return std::string();
    }
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    char* buf = (char*)malloc(sizeof(char) * (len + 1));
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buf, len, NULL, NULL);
    buf[len] = '\0';
    std::string str(buf);
    free(buf);
    return str;
}
#endif

/**
 * @brief 获取程序文件
 * @return 程序文件全路径
 */
std::string getProcFile()
{
#if _WIN32
#ifdef UNICODE
    TCHAR exeFileW[MAX_PATH + 1] = {0};
    GetModuleFileName(NULL, exeFileW, MAX_PATH);
    return wstring2string(exeFileW);
#else
    CHAR exeFile[MAX_PATH + 1] = {0};
    GetModuleFileName(NULL, exeFile, MAX_PATH);
    return exeFile;
#endif
#else
    char exeFile[261] = {0};
    unsigned int exeFileLen = readlink("/proc/self/exe", exeFile, sizeof(exeFile) - 1);
    if (exeFileLen <= 0 || exeFileLen >= sizeof(exeFile) - 1)
    {
        return std::string();
    }
    exeFile[exeFileLen] = '\0';
    return exeFile;
#endif
}

/*********************************************************************
 业务逻辑
*********************************************************************/
std::string g_procFile; /* 当前程序文件全路径名 */
std::string g_procBasename; /* 当前程序文件基础名 */
std::string g_outputPath; /* 崩溃堆栈文件输出路径 */
DumpCallback g_callback = nullptr; /* 崩溃回调 */
google_breakpad::ExceptionHandler* g_execptionHandler = nullptr; /* 异常句柄 */

/**
 * @brief 崩溃处理回调
 */
bool dumpHandler(const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded)
{
    /* 获取当前时间戳 */
    time_t now;
    time(&now);
    struct tm t = *localtime(&now);
    char datetime[16] = {0};
    strftime(datetime, sizeof(datetime), "%Y%m%d%H%M%S", &t);
    struct timeb tb;
    ftime(&tb);
    char ms[4] = {0};
    sprintf(ms, "%03d", tb.millitm);
    /* 重命名堆栈文件 */
    auto fi = stripFileInfo(descriptor.path());
    std::string dumpFile = fi[0] + g_procBasename + "_" + datetime + ms + (fi[3].empty() ? "" : "." + fi[3]);
    std::string command = "mv " + std::string(descriptor.path()) + " " + dumpFile;
    std::vector<std::string> result;
    int ret = shellCmd(command, &result);
    if (0 != ret || !result.empty())
    {
        dumpFile = descriptor.path();
    }
    /* 堆栈文件处理 */
    command = "dump_assist.sh -pname " + g_procFile + " -dname " + dumpFile;
    ret = shellCmd(command, &result);
    /* 回调到外部 */
    if (g_callback)
    {
        std::string json;
        if (!result.empty())
        {
            json = result[result.size() - 1];
        }
        g_callback(json);
    }
    return succeeded;
}

void start(const std::string& outputPath, const DumpCallback& callback)
{
    static bool s_started = false;
    if (s_started)
    {
        return;
    }
    s_started = true;
    auto procFile = getProcFile();
    auto fileInfo = stripFileInfo(procFile);
    /* 参数保存 */
    g_procFile = procFile;
    g_procBasename = fileInfo[2];
    g_outputPath = outputPath.empty() ? fileInfo[0] + "dump" : outputPath;
    g_callback = callback;
    /* 创建路径 */
    createPath(g_outputPath);
    /* 开始监听 */
    google_breakpad::MinidumpDescriptor descriptor(g_outputPath.c_str());
    g_execptionHandler = new google_breakpad::ExceptionHandler(descriptor, NULL, dumpHandler, NULL, true, -1);
}
} // namespace crashdump
