#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utility/mmfile/mmfile.h"

void testMmfile()
{
    printf("\n============================== test mmfile =============================\n");
    std::string fileName = "testMmfile.dat";
    /* ������д������ */
    {
        utility::MMFile file;
        if (!file.open(fileName, utility::MMFile::AccessMode::create, 1024 * 1024, 1024 * 1024)) /* ����һ����ʼ��СΪ 1MB ���ļ� */
        {
            std::cerr << "Failed to open file for writing: " << file.getLastError() << std::endl;
            return;
        }
        /* д������ */
        const char* data = "Hello, Memory Mapped File!";
        size_t dataSize = strlen(data);
        size_t written = file.write(data, dataSize);
        if (written != dataSize)
        {
            std::cerr << "Failed to write data: " << file.getLastError() << std::endl;
            return;
        }
        std::cout << "Data written successfully." << std::endl;
        /* �ر��ļ� */
        file.close();
    }
    /* ��ȡ���� */
    {
        utility::MMFile file;
        if (!file.open(fileName, utility::MMFile::AccessMode::read_only))
        {
            std::cerr << "Failed to open file for reading: " << file.getLastError() << std::endl;
            return;
        }
        /* ��λ���ļ���ͷ */
        if (!file.seek(0, SEEK_SET))
        {
            std::cerr << "Failed to seek to file start: " << file.getLastError() << std::endl;
            return;
        }
        /* ��ȡ���� */
        char buffer[100] = {0};
        size_t readedSize = 0;
        if (!file.read(sizeof(buffer) - 1, [&](const void* data, size_t count) {
                memcpy(buffer + readedSize, data, count);
                readedSize += count;
                buffer[readedSize] = '\0'; /* ȷ���ַ�����β */
            }))
        {
            std::cerr << "Failed to read data: " << file.getLastError() << std::endl;
            return;
        }
        std::cout << "Data read from file: " << buffer << std::endl;
        /* �ر��ļ� */
        file.close();
    }
    /* ��֤�ļ����� */
    {
        utility::MMFile file;
        if (!file.open(fileName, utility::MMFile::AccessMode::read_only))
        {
            std::cerr << "Failed to open file for verification: " << file.getLastError() << std::endl;
            return;
        }
        /* ��ȡ�����ļ����� */
        char* buffer = new char[file.getFileSize()];
        size_t readedSize = 0;
        if (!file.read(sizeof(buffer) - 1, [&](const void* data, size_t count) {
                memcpy(buffer + readedSize, data, count);
                readedSize += count;
            }))
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
