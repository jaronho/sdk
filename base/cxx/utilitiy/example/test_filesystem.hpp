#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utilitiy/filesystem/file_info.h"
#include "../utilitiy/filesystem/path_info.h"

void testFilesystem()
{
    printf("\n============================== test filesystem =============================\n");
    printf("---------- file info\n");
#ifdef _WIN32
    utilitiy::FileInfo fi("D:\\config.txt");
#else
    utilitiy::FileInfo fi("/home/jaron/config.txt");
#endif
    printf("name: %s\n", fi.name().c_str());
    printf("path: %s\n", fi.path().c_str());
    printf("filename: %s\n", fi.filename().c_str());
    printf("basename: %s\n", fi.basename().c_str());
    printf("extname: %s\n", fi.extname().c_str());
    if (fi.exist())
    {
        printf("exist: true\n");
        long long fileSize;
        char* fileData = fi.data(fileSize, true);
        printf("size: %lld\n", fileSize);
        if (fileData)
        {
            printf("data:--------------------\n%s\n--------------------\n", fileData);
            free(fileData);
        }
        long long offset = 2;
        long long count = 4;
        char* buffer = fi.read(offset, count);
        if (buffer)
        {
            printf("offset[%lld], count[%lld], buffer:--------------------\n", offset, count);
            for (size_t i = 0; i < count; ++i)
            {
                printf("%c", buffer[i]);
            }
            printf("\n--------------------\n");
            free(buffer);
        }
        utilitiy::FileAttribute attr = fi.attribute();
        printf("- createTime: %ld\n", attr.createTime);
        printf("- modifyTime: %ld\n", attr.modifyTime);
        printf("- accessTime: %ld\n", attr.accessTime);
        printf("- size: %lld\n", attr.size);
        printf("- isDir: %s\n", attr.isDir ? "true" : "false");
        printf("- isFile: %s\n", attr.isFile ? "true" : "false");
#ifdef _WIN32
        printf("- isSystem: %s\n", attr.isSystem ? "true" : "false");
#endif
        printf("- isSymLink: %s\n", attr.isSymLink ? "true" : "false");
        printf("- isWritable: %s\n", attr.isWritable ? "true" : "false");
        printf("- isExecutable: %s\n", attr.isExecutable ? "true" : "false");
        printf("- isHidden: %s\n", attr.isHidden ? "true" : "false");
    }
    else
    {
        printf("exist: false\n");
    }
    printf("---------- path info\n");
#ifdef _WIN32
    utilitiy::PathInfo pi("D:\\workspace\\test\\111\\222\\333");
#else
    utilitiy::PathInfo pi("/home/jaron/test/111/222/333");
#endif
    printf("path: %s\n", pi.path().c_str());
    if (pi.exist())
    {
        printf("exist: true\n");
        utilitiy::FileAttribute attr = pi.attribute();
        printf("- createTime: %ld\n", attr.createTime);
        printf("- modifyTime: %ld\n", attr.modifyTime);
        printf("- accessTime: %ld\n", attr.accessTime);
        printf("- size: %lld\n", attr.size);
        printf("- isDir: %s\n", attr.isDir ? "true" : "false");
        printf("- isFile: %s\n", attr.isFile ? "true" : "false");
#ifdef _WIN32
        printf("- isSystem: %s\n", attr.isSystem ? "true" : "false");
#endif
        printf("- isSymLink: %s\n", attr.isSymLink ? "true" : "false");
        printf("- isWritable: %s\n", attr.isWritable ? "true" : "false");
        printf("- isExecutable: %s\n", attr.isExecutable ? "true" : "false");
        printf("- isHidden: %s\n", attr.isHidden ? "true" : "false");
        if (pi.clear())
        {
            printf("clear: true\n");
        }
        else
        {
            printf("clear: false\n");
        }
    }
    else
    {
        printf("exist: false\n");
        if (pi.create())
        {
            printf("create: true\n");
            utilitiy::FileAttribute attr = pi.attribute();
            printf("- createTime: %ld\n", attr.createTime);
            printf("- modifyTime: %ld\n", attr.modifyTime);
            printf("- accessTime: %ld\n", attr.accessTime);
            printf("- isDir: %s\n", attr.isDir ? "true" : "false");
            printf("- isFile: %s\n", attr.isFile ? "true" : "false");
#ifdef _WIN32
            printf("- isSystem: %s\n", attr.isSystem ? "true" : "false");
#endif
            printf("- isSymLink: %s\n", attr.isSymLink ? "true" : "false");
            printf("- isWritable: %s\n", attr.isWritable ? "true" : "false");
            printf("- isExecutable: %s\n", attr.isExecutable ? "true" : "false");
            printf("- isHidden: %s\n", attr.isHidden ? "true" : "false");
            if (pi.remove())
            {
                printf("remove: true\n");
            }
            else
            {
                printf("remove: false\n");
            }
        }
        else
        {
            printf("create: false\n");
        }
    }
}