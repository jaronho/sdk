#include "system.h"

#include <algorithm>
#include <chrono>
#include <climits>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <thread>
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#endif

namespace utility
{

int System::runCmd(const std::string& cmd, const std::function<void()>& startedFunc,
                   const std::function<bool(const char* data, size_t count)>& outputFunc, unsigned int timeoutMS)
{
    if (cmd.empty())
    {
        return -1;
    }
#ifdef _WIN32
    FILE* stream = _popen(cmd.c_str(), "r");
#else
    FILE* stream = popen(cmd.c_str(), "r");
#endif
    if (!stream)
    {
        return -2;
    }
    if (startedFunc)
    {
        startedFunc(); /* 创建管道/子进程成功回调, 用于调用方感知命令已启动 */
    }
#ifdef _WIN32
    int fd = _fileno(stream);
#else
    int fd = fileno(stream);
#endif
    bool timeoutFlag = (fd < 0); /* fd无效: 无法读写管道, 按放弃等待处理, 避免pclose永久阻塞 */
    bool stopFlag = false; /* 区分"回调要求停止"与"超时", 便于调用方判断 */
    const int abandonCode = timeoutFlag ? -3 : -4; /* fd无效返回-3, 超时返回-4 */
    if (!timeoutFlag) /* 必须抽干管道: 即使不需要输出, 不读会让子进程写满管道缓冲后阻塞在write上, 与pclose互等死锁 */
    {
        auto startTime = std::chrono::steady_clock::now(); /* 计时起点: 从开始读管道算起, 不含popen耗时 */
        char data[4096] = {0};
        while (1)
        {
            if (timeoutMS > 0) /* 等待管道可读, 超过剩余时间则视为超时 */
            {
                auto usedMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
                if (usedMS >= timeoutMS) /* long long与unsigned int比较, 会自动提升为long long */
                {
                    timeoutFlag = true;
                    break;
                }
#ifdef _WIN32
                DWORD available = 0;
                if (!PeekNamedPipe((HANDLE)_get_osfhandle(fd), NULL, 0, NULL, &available, NULL)) /* 探测失败 */
                {
                    /* 写端关闭(命令正常结束)走普通收尾; 其他异常说明子进程状态未知, 放弃等待避免_pclose永久阻塞 */
                    auto peekErr = GetLastError();
                    timeoutFlag = (ERROR_BROKEN_PIPE != peekErr && ERROR_PIPE_NOT_CONNECTED != peekErr);
                    break;
                }
                if (0 == available) /* 暂无数据: 等待至多10ms, 且不超过剩余超时时间 */
                {
                    auto remainMS = (long long)(timeoutMS)-usedMS;
                    Sleep((DWORD)(remainMS > 10 ? 10 : remainMS));
                    continue;
                }
#else
                struct pollfd pfd = {fd, POLLIN, 0};
                auto waitMS = std::min<long long>(timeoutMS - usedMS, INT_MAX); /* 截断到int上限, 避免timeoutMS过大时强转成负数 */
                auto pollRet = poll(&pfd, 1, (int)(waitMS));
                if (pollRet < 0)
                {
                    if (EINTR == errno) /* 被信号打断: 重试 */
                    {
                        continue;
                    }
                    timeoutFlag = true; /* 等待异常: 子进程可能仍在运行, 放弃等待 */
                    break;
                }
                if (0 == pollRet || (pfd.revents & (POLLERR | POLLNVAL))) /* 超时或管道异常: 放弃等待 */
                {
                    timeoutFlag = true;
                    break;
                }
#endif
            }
            auto count = read(fd, data, sizeof(data));
            if (0 == count) /* 管道关闭, 命令已结束 */
            {
                break;
            }
            else if (count < 0) /* 读取失败 */
            {
                if (EAGAIN == errno || EWOULDBLOCK == errno || EINTR == errno) /* 无数据或被信号打断: 重试 */
                {
                    continue;
                }
#ifdef _WIN32
                /* 管道仍连接则子进程状态未知, 放弃等待 */
                DWORD available = 0;
                timeoutFlag = FALSE != PeekNamedPipe((HANDLE)_get_osfhandle(fd), NULL, 0, NULL, &available, NULL);
#else
                timeoutFlag = true; /* 子进程可能仍在运行, 放弃等待 */
#endif
                break;
            }
            if (outputFunc && !outputFunc(data, count))
            {
                stopFlag = true;
                break;
            }
        }
    }
    if (timeoutFlag || stopFlag) /* 放弃等待: 只关闭管道, 不等待也不结束子进程, 超时/停止后子进程可能仍在运行 */
    {
        if (fd >= 0) /* fd无效说明stream本身异常: 此时释放只会出错, 只能放弃(泄漏一份句柄) */
        {
#ifdef _WIN32
            fclose(stream); /* 只关闭管道, 避免_pclose永久阻塞; CRT中的进程句柄泄漏一份, 由进程退出时回收 */
#else
            close(fd); /* 只关闭fd放弃等待, 避免fclose内部waitpid卡死; FILE泄漏一份, 由进程退出时回收 */
#endif
        }
        if (stopFlag) /* 主动停止不是错误: 清掉可能残留的errno, 避免调用方误读 */
        {
            errno = 0;
        }
        else if (fd >= 0) /* 仅超时/管道异常才设置超时errno, fd无效不是超时 */
        {
            errno = ETIMEDOUT;
        }
        return stopFlag ? -5 : abandonCode; /* -5-回调主动停止, 便于与超时区分 */
    }
#ifdef _WIN32
    return _pclose(stream);
#else
    errno = 0;
    int ret = pclose(stream);
    if (-1 == ret && ECHILD == errno) /* 当进程某处设置了signal(SIGCHLD, SIG_IGN)时, 会出现"No child processes", 这里就设置不认为出错 */
    {
        ret = 0;
    }
    return ret;
#endif
}

int System::runCmd(const std::string& cmd, std::string* outStr, std::vector<std::string>* outVec, bool ignoreBlankLine,
                   unsigned int timeoutMS)
{
    if (outStr)
    {
        outStr->clear();
    }
    if (outVec)
    {
        outVec->clear();
    }
    std::string line; /* outStr为空时的"未成行"缓存 */
    size_t lineStart = 0; /* outStr非空时"未成行"部分的起点: 直接复用outStr, 省掉一份完整副本 */
    auto ret = runCmd(
        cmd, nullptr,
        [&](const char* data, size_t count) {
            if (outStr)
            {
                outStr->append(data, count);
            }
            if (!outVec) /* 只要整串, 无需切行 */
            {
                return true;
            }
            if (!outStr)
            {
                line.append(data, count);
            }
            const std::string& src = outStr ? *outStr : line;
            size_t start = lineStart;
            while (1)
            {
                size_t pos = src.find('\n', start);
                if (std::string::npos == pos)
                {
                    break;
                }
                size_t end = (pos > start && '\r' == src[pos - 1]) ? pos - 1 : pos; /* 兼容\r\n */
                if (end > start || !ignoreBlankLine)
                {
                    outVec->emplace_back(src, start, end - start);
                }
                start = pos + 1;
            }
            if (outStr)
            {
                lineStart = start; /* outStr是累积的, 绝对索引可跨chunk使用 */
            }
            else
            {
                line.erase(0, start); /* 已切分的部分一次性移除 */
            }
            return true;
        },
        timeoutMS);
    if (outVec) /* 末尾无换行的残余内容 */
    {
        const std::string& tail = outStr ? *outStr : line;
        size_t start = outStr ? lineStart : 0;
        size_t end = tail.size();
        if (end > start && '\r' == tail[end - 1]) /* 丢弃被chunk拆开的\r */
        {
            --end;
        }
        if (end > start)
        {
            outVec->emplace_back(tail, start, end - start);
        }
    }
    return ret;
}

bool System::tryLockUnlockFile(HANDLE fd, bool lock, bool block)
{
    if (fd <= 0) /* 文件不存在 */
    {
        return false;
    }
#ifdef _WIN32
    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(OVERLAPPED));
    if (lock)
    {
        if (0 == LockFileEx(fd, block ? LOCKFILE_EXCLUSIVE_LOCK : LOCKFILE_FAIL_IMMEDIATELY, (DWORD)0, (DWORD)0, (DWORD)0, &overlapped))
        {
            return false;
        }
    }
    else
    {
        if (0 == UnlockFileEx(fd, (DWORD)0, (DWORD)0, (DWORD)0, &overlapped))
        {
            return false;
        }
    }
#else
    int lockType = lock ? F_WRLCK : F_UNLCK;
    struct flock fl;
    fl.l_type = lockType; /* 锁的类型 */
    fl.l_whence = SEEK_SET; /* 偏移量的起始位置 */
    fl.l_start = 0; /* 加锁的起始偏移 */
    fl.l_len = 0; /* 上锁的字节数, 为0时表示锁的区域从起点开始直至最大的可能位置 */
    int ret = fcntl(fd, block ? F_SETLKW : F_SETLK, &fl);
    if (0 != ret || lockType != fl.l_type)
    {
        return false;
    }
#endif
    return true;
}

bool System::tryLockFile(const std::string& filename, bool block)
{
    if (filename.empty())
    {
        return false;
    }
    /* 打开要加锁的文件(注意: 这里文件句柄不做关闭处理) */
#ifdef _WIN32
    HANDLE fd = CreateFileA(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, (DWORD)0, NULL);
    if (!fd)
    {
        return false;
    }
#else
    int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0777);
    if (-1 == fd)
    {
        return false;
    }
#endif
    /* 加锁 */
    return tryLockUnlockFile(fd, true, block);
}

bool System::tryLockFileTemporary(const std::string& filename, const std::function<void(bool lockOk)>& func, const std::string& suffix)
{
    if (filename.empty())
    {
        return false;
    }
    std::string fileLockName = filename + (suffix.empty() ? ".lock" : ('.' == suffix[0] ? suffix : ("." + suffix)));
    /* 打开要加锁的文件 */
#ifdef _WIN32
    HANDLE fd = CreateFileA(fileLockName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, (DWORD)0, NULL);
    if (!fd)
    {
        return false;
    }
#else
    int fd = open(fileLockName.c_str(), O_RDWR | O_CREAT, 0777);
    if (-1 == fd)
    {
        return false;
    }
#endif
    /* 加锁 */
    auto lockOk = tryLockUnlockFile(fd, true, true);
    /* 执行逻辑 */
    if (func)
    {
        func(lockOk);
    }
    /* 解锁 */
    tryLockUnlockFile(fd, false, true);
    /* 关闭加锁文件 */
#ifdef _WIN32
    CloseHandle(fd);
#else
    close(fd);
#endif
    return true;
}

#ifdef _WIN32
bool System::checkFileLock(HANDLE fd)
#else
bool System::checkFileLock(int fd)
#endif
{
    if (fd <= 0) /* 文件不存在 */
    {
        return false;
    }
#ifdef _WIN32
    /* 尝试解锁 */
    tryLockUnlockFile(fd, false, false);
    /* 获取错误码, 参考: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499- */
    if (0x9E == GetLastError()) /* The segment is already unlocked */
    {
        return false;
    }
#else
    struct flock fl;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fcntl(fd, F_GETLK, &fl);
    if (F_UNLCK == fl.l_type)
    {
        return false;
    }
#endif
    return true;
}

bool System::checkFileLock(const std::string& filename)
{
    bool ret = false;
#ifdef _WIN32
    HANDLE fd = CreateFileA(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, (DWORD)0, NULL);
    if (fd)
    {
        ret = checkFileLock(fd);
        CloseHandle(fd);
    }
#else
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd > 0)
    {
        ret = checkFileLock(fd);
        close(fd);
    }
#endif
    return ret;
}

void System::waitForTime(unsigned int maxMS, const std::function<bool()>& func, unsigned int loopGap)
{
    if (0 == maxMS)
    {
        if (func)
        {
            func();
        }
    }
    else
    {
        if (func)
        {
            auto endTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(maxMS);
            while (std::chrono::steady_clock::now() < endTime)
            {
                if (func())
                {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(loopGap));
            }
        }
        else /* 无执行函数, 则作为等待接口调用 */
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(maxMS));
        }
    }
}

void System::waitForCount(unsigned int maxCount, const std::function<bool()>& func, unsigned int loopGap)
{
    if (0 == maxCount)
    {
        if (func)
        {
            func();
        }
    }
    else
    {
        if (func)
        {
            unsigned int count = 0;
            while (count < maxCount)
            {
                if (func())
                {
                    break;
                }
                ++count;
                std::this_thread::sleep_for(std::chrono::milliseconds(loopGap));
            }
        }
        else /* 无执行函数, 则作为等待接口调用 */
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(maxCount * loopGap));
        }
    }
}

std::string System::getHostname()
{
#ifdef _WIN32
    CHAR hostname[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD hostnameLen = MAX_COMPUTERNAME_LENGTH;
    GetComputerNameA(hostname, &hostnameLen);
    return hostname;
#else
    char hostname[32] = {0};
    gethostname(hostname, sizeof(hostname));
    return hostname;
#endif
}

#ifdef _WIN32
bool System::isRunAsAdmin()
{
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = nullptr;
    /* 创建"Administrators"组的SID */
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                 &administratorsGroup))
    {
        CheckTokenMembership(NULL, administratorsGroup, &isAdmin); /* 检查当前线程是否属于该组 */
        FreeSid(administratorsGroup);
    }
    return (TRUE == isAdmin);
}
#endif
} // namespace utility
