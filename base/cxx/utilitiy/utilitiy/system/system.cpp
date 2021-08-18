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
int System::tryLockFile(int fd, bool lock)
{
    struct flock fl;
    fl.l_type = lock ? F_WRLCK : F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl);
}

int System::tryLockFile(const std::string& filename, bool lock)
{
    int fd = open(filename.c_str(), O_RDWR, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if (fd < 0) /* 文件不存在 */
    {
        return -1;
    }
    int ret = tryLockFile(fd, lock);
    close(fd);
    return ret;
}

int System::getFileLockStatus(const std::string& filename)
{
    int fd = open(filename.c_str(), O_RDWR, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if (fd < 0) /* 文件不存在 */
    {
        return 1;
    }
    if (-1 == tryLockFile(fd, true)) /* 文件加锁失败 */
    {
        close(fd);
        return 2;
    }
    tryLockFile(fd, false); /* 此处要注意记得解锁和关闭文件 */
    close(fd);
    return 3;
}
#endif
} // namespace utilitiy
