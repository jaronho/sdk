#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utility/mmfile/mmfile.h"

void testMmfile()
{
    printf("\n============================== test mmfile =============================\n");
    std::string fileName = "testMmfile.dat";
    /* 创建并写入数据 */
    {
        utility::MMFile file;
        if (!file.open(fileName, utility::MMFile::AccessMode::create, 1024 * 1024, 1024 * 1024)) /* 创建一个初始大小为 1MB 的文件 */
        {
            std::cerr << "Failed to open file for writing: " << file.getLastError() << std::endl;
            return;
        }
        /* 写入数据 */
        const char* data = "Hello, Memory Mapped File!";
        size_t dataSize = strlen(data);
        size_t written = file.write(data, dataSize);
        if (written != dataSize)
        {
            std::cerr << "Failed to write data: " << file.getLastError() << std::endl;
            return;
        }
        std::cout << "Data written successfully." << std::endl;
        /* 关闭文件 */
        file.close();
    }
    /* 读取数据 */
    {
        utility::MMFile file;
        if (!file.open(fileName, utility::MMFile::AccessMode::read_only))
        {
            std::cerr << "Failed to open file for reading: " << file.getLastError() << std::endl;
            return;
        }
        /* 定位到文件开头 */
        if (!file.seek(0, SEEK_SET))
        {
            std::cerr << "Failed to seek to file start: " << file.getLastError() << std::endl;
            return;
        }
        /* 读取数据 */
        char buffer[100];
        size_t readSize = file.read(buffer, sizeof(buffer) - 1);
        if (readSize == 0)
        {
            std::cerr << "Failed to read data: " << file.getLastError() << std::endl;
            return;
        }
        buffer[readSize] = '\0'; /* 确保字符串结尾 */
        std::cout << "Data read from file: " << buffer << std::endl;
        /* 关闭文件 */
        file.close();
    }
    /* 验证文件内容 */
    {
        utility::MMFile file;
        if (!file.open(fileName, utility::MMFile::AccessMode::read_only))
        {
            std::cerr << "Failed to open file for verification: " << file.getLastError() << std::endl;
            return;
        }
        /* 读取整个文件内容 */
        char* buffer = new char[file.getFileSize()];
        size_t readSize = file.read(buffer, file.getFileSize());
        if (readSize != file.getFileSize())
        {
            std::cerr << "Failed to read entire file: " << file.getLastError() << std::endl;
            delete[] buffer;
            return;
        }
        std::cout << "File content:" << std::endl;
        std::cout << buffer << std::endl;
        delete[] buffer;
        file.close();
    }
    std::cout << "All operations completed successfully." << std::endl;
}
