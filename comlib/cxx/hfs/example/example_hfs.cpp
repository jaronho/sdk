#include <iostream>
#include <thread>

#include "../hfs/http_file_server.h"

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is Http File Server                                                                              **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   address, default: 0.0.0.0                                                      **\n");
    printf("** [-p]                   port, default: 4335                                                            **\n");
    printf("** [-d]                   file dir, default: program directory                                           **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-ssl]                 specify enable ssl [0-disable, 1-enable]. default: 0                           **\n");
    printf("** [-way]                 specify ssl way verify [1, 2], default: 1                                      **\n");
    printf("** [-fmt]                 specify file format [1-DER, 2-PEM]. default: 2                                 **\n");
    printf("** [-cf]                  specify certificate file. e.g. server.crt                                      **\n");
    printf("** [-pkf]                 specify private key file, e.g. server.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string host;
    int port = 0;
    std::string filePath;
    int sslOn = 0;
    int sslWay = 1;
    int certFmt = 2;
    std::string certFile;
    std::string privateKeyFile;
    std::string privateKeyFilePwd;
    for (int i = 1; i < argc;)
    {
        const char* key = argv[i];
        if (0 == strcmp(key, "-s")) /* 服务地址 */
        {
            ++i;
            if (i < argc)
            {
                host = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-p")) /* 端口 */
        {
            ++i;
            if (i < argc)
            {
                port = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-d")) /* 端口 */
        {
            ++i;
            if (i < argc)
            {
                filePath = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-ssl")) /* 是否启用SSL */
        {
            ++i;
            if (i < argc)
            {
                sslOn = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-way")) /* SSL校验 */
        {
            ++i;
            if (i < argc)
            {
                sslWay = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-fmt")) /* 文件格式 */
        {
            ++i;
            if (i < argc)
            {
                certFmt = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-cf")) /* 证书文件 */
        {
            ++i;
            if (i < argc)
            {
                certFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkf")) /* 私钥文件 */
        {
            ++i;
            if (i < argc)
            {
                privateKeyFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkp")) /* 私钥文件密码 */
        {
            ++i;
            if (i < argc)
            {
                privateKeyFilePwd = argv[i];
                ++i;
            }
        }
        else
        {
            ++i;
        }
    }
    if (host.empty())
    {
        host = "0.0.0.0";
    }
    if (port <= 0)
    {
        port = 4335;
    }
    if (sslOn < 0)
    {
        sslOn = 0;
    }
    else if (sslOn > 1)
    {
        sslOn = 1;
    }
    if (sslWay < 1)
    {
        sslWay = 1;
    }
    else if (sslWay > 2)
    {
        sslWay = 2;
    }
    if (certFmt < 1)
    {
        certFmt = 1;
    }
    else if (certFmt > 2)
    {
        certFmt = 2;
    }
    printf("server: %s:%d, ssl: %d, way: %d, certFmt: %d, certFile: %s, pkFile: %s\n", host.c_str(), port, sslOn, sslWay, certFmt,
           certFile.c_str(), privateKeyFile.c_str());
    auto server = std::make_shared<hfs::HttpFileServer>("hfs", 6, host, port, filePath);
    printf("path: %s\n", server->getRootDir().c_str());
    server->run();
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
