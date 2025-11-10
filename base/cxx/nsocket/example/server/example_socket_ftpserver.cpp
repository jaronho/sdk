#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>

#include "../../nsocket/ftp/server.h"

int main(int argc, char* argv[])
{
    printf("***********************************************************************************************************\n");
    printf("** This is Ftp server                                                                                    **\n");
    printf("** Options:                                                                                              **\n");
    printf("**                                                                                                       **\n");
    printf("** [-s]                   server address, default: 0.0.0.0                                               **\n");
    printf("** [-p]                   server port, default: 21                                                       **\n");
    printf("** [-d]                   file dir, default: program directory                                           **\n");
#if (1 == ENABLE_NSOCKET_OPENSSL)
    printf("** [-tls]                 specify enable ssl [0-disable, 1-enable]. default: 0                           **\n");
    printf("** [-way]                 specify ssl way verify [1, 2], default: 1                                      **\n");
    printf("** [-pem]                 specify file format [1-DER, 2-PEM]. default: 2                                 **\n");
    printf("** [-cf]                  specify certificate file. e.g. client.crt, ca.crt                              **\n");
    printf("** [-pkf]                 specify private key file, e.g. client.key                                      **\n");
    printf("** [-pkp]                 specify private key file password, e.g. qq123456                               **\n");
#endif
    printf("**                                                                                                       **\n");
    printf("***********************************************************************************************************\n");
    printf("\n");
    std::string serverHost;
    int serverPort = 0;
    std::string rootPath;
    int sslOn = 0;
    int sslWay = 1;
    int certFmt = 2;
    std::string certFile;
    std::string pkFile;
    std::string pkPwd;
    for (int i = 1; i < argc;)
    {
        const char* key = argv[i];
        if (0 == strcmp(key, "-s")) /* 服务器地址 */
        {
            ++i;
            if (i < argc)
            {
                serverHost = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-p")) /* 服务器端口 */
        {
            ++i;
            if (i < argc)
            {
                serverPort = atoi(argv[i]);
                ++i;
            }
        }
        else if (0 == strcmp(key, "-d")) /* 根路径 */
        {
            ++i;
            if (i < argc)
            {
                rootPath = argv[i];
                ++i;
            }
        }
#if (1 == ENABLE_NSOCKET_OPENSSL)
        else if (0 == strcmp(key, "-tls")) /* 是否启用TLS */
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
        else if (0 == strcmp(key, "-pem")) /* 文件格式 */
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
                pkFile = argv[i];
                ++i;
            }
        }
        else if (0 == strcmp(key, "-pkp")) /* 私钥文件密码 */
        {
            ++i;
            if (i < argc)
            {
                pkPwd = argv[i];
                ++i;
            }
        }
#endif
        else
        {
            ++i;
        }
    }
    if (serverHost.empty())
    {
        serverHost = "0.0.0.0";
    }
    if (serverPort <= 0 || serverPort > 65535)
    {
        serverPort = 21;
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
    if (rootPath.empty())
    {
        std::string fullFilePath(argv[0]);
        auto pos = fullFilePath.find_last_of("/\\");
        if (pos < fullFilePath.size())
        {
            rootPath = fullFilePath.substr(0, pos + 1);
        }
        else
        {
            rootPath = fullFilePath;
        }
    }
    auto server = std::make_shared<nsocket::ftp::Server>("ftp_server", 10, serverHost, serverPort);
    server->setRootPath(rootPath);
    /* 注意: 最好增加异常捕获, 因为当密码不对时会抛异常 */
    try
    {
        if (1 == sslOn && !certFile.empty() && !pkFile.empty())
        {
            printf("server: %s:%d, ssl way: %d, certFile: %s, pkFile: %s\n", serverHost.c_str(), serverPort, sslWay, certFile.c_str(),
                   pkFile.c_str());
        }
        else
        {
            printf("server: %s:%d, rootPath: %s\n", serverHost.c_str(), serverPort, rootPath.c_str());
        }
        std::string errDesc;
        if (server->run(sslOn, sslWay, certFmt, certFile, pkFile, pkPwd, &errDesc))
        {
            while (1)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        else
        {
            printf("server run fail: %s\n", errDesc.c_str());
        }
    }
    catch (const std::exception& e)
    {
        printf("========== execption: %s\n", e.what());
    }
    catch (...)
    {
        printf("========== execption: unknown\n");
    }
    return 0;
}
