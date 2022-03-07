#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../utility/filesystem/file_info.h"
#include "../utility/filesystem/path_info.h"

void testFilesystem()
{
    printf("\n============================== test filesystem =============================\n");
    printf("---------- file info\n");
#ifdef _WIN32
    utility::FileInfo fi("D:\\config.txt");
#else
    utility::FileInfo fi("/home/jaron/config.txt");
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
        size_t offset = 2;
        size_t count = 4;
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
        utility::FileAttribute attr = fi.attribute();
        printf("- createTime: %s\n", attr.createTimeFmt().c_str());
        printf("- modifyTime: %s\n", attr.modifyTimeFmt().c_str());
        printf("- accessTime: %s\n", attr.accessTimeFmt().c_str());
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
    printf("---------- path info, current: %s\n", utility::PathInfo::getcwd().c_str());
#ifdef _WIN32
    utility::PathInfo pi("D:\\workspace\\test\\111\\222\\333");
#else
    utility::PathInfo pi("/home/jaron/test/111/222/333");
#endif
    printf("path: %s\n", pi.path().c_str());
    if (pi.exist())
    {
        printf("exist: true\n");
        utility::FileAttribute attr = pi.attribute();
        printf("- createTime: %s\n", attr.createTimeFmt().c_str());
        printf("- modifyTime: %s\n", attr.modifyTimeFmt().c_str());
        printf("- accessTime: %s\n", attr.accessTimeFmt().c_str());
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
            utility::FileAttribute attr = pi.attribute();
            printf("- createTime: %s\n", attr.createTimeFmt().c_str());
            printf("- modifyTime: %s\n", attr.modifyTimeFmt().c_str());
            printf("- accessTime: %s\n", attr.accessTimeFmt().c_str());
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
