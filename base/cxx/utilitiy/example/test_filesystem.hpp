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
    utilitiy::FileInfo fi("D:\\config.json");
#else
    utilitiy::FileInfo fi("/home/jaron/config.json");
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
    printf("path: %s\n", fi.path().c_str());
    if (pi.exist())
    {
        printf("exist: true\n");
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
