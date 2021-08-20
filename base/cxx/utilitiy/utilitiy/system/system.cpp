#include "system.h"

#include <string.h>

#ifdef _WIN32
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace utilitiy
{
int System::runCmd(const std::string& cmd, std::string* outStr, std::vector<std::string>* outVec)
{
    if (outStr)
    {
        (*outStr).clear();
    }
    if (outVec)
    {
        (*outVec).clear();
    }
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
    if (outStr || outVec)
    {
        const size_t bufferSize = 1024;
        char buffer[bufferSize] = {0};
        std::string line;
        while (memset(buffer, 0, bufferSize) && fgets(buffer, bufferSize - 1, stream))
        {
            if (outStr)
            {
                (*outStr).append(buffer);
            }
            if (outVec)
            {
                line += buffer;
                size_t pos = line.find('\n');
                if (std::string::npos != pos)
                {
                    (*outVec).emplace_back(line.substr(0, pos));
                    line = line.substr(pos + 1, line.size() - pos);
                }
            }
        }
    }
#ifdef _WIN32
    return _pclose(stream);
#else
    return pclose(stream);
#endif
}

#ifndef _WIN32
bool System::tryLockFile(int fd, bool lock, bool block)
{
    int lockType = lock ? F_WRLCK : F_UNLCK;
    struct flock fl;
    fl.l_type = lockType; /* 锁的类型 */
    fl.l_whence = SEEK_SET; /* 偏移量的起始位置 */
    fl.l_start = 0; /* 加锁的起始偏移 */
    fl.l_len = 0; /* 上锁的字节数, 为0时表示锁的区域从起点开始直至最大的可能位置 */
    int ret = fcntl(fd, block ? F_SETLKW : F_SETLK, &fl);
    if (0 == ret && lockType == fl.l_type)
    {
        return true;
    }
    return false;
}

bool System::checkFileLock(int fd)
{
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
    return true;
}

bool System::checkFileLock(const std::string& filename)
{
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0) /* 文件不存在 */
    {
        return false;
    }
    bool ret = checkFileLock(fd);
    close(fd);
    return ret;
}
#endif
} // namespace utilitiy
