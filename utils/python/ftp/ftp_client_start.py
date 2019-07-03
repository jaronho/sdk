#!/usr/bin/python2
# -*- coding: UTF-8 -*-
from sdk import kill_process
from ftplib import FTP
import argparse
import json
import math
import os
import re
import sys

""" FTP登录 """
def ftpLogin(host, port, user="anonymous", password="", pasv=True):                          
    ftp = FTP()
    try:
        ftp.set_pasv(pasv)      # True.被动模式,False.主动模式
        ftp.connect(host, port)
        ftp.encoding = "utf-8"
    except:
        print(sys.exc_info())
    else:
        try:
            ftp.login(user, password)
        except:
            print(sys.exc_info())
        else:
            return ftp

""" FTP目录递归下载 """
def ftpDownload(ftp, remotePath, localPath, filterBeforeCB=None, filterAfterCB=None):
    # step1: 对路径补位分隔符
    if False == remotePath.endswith("/"):
        remotePath += "/"
    if False == localPath.endswith("/"):
        localPath += "/"
    # step2: 切换目录
    try:
        ftp.cwd(remotePath)
        if False == os.path.exists(localPath):
            os.makedirs(localPath)
    except:
        print(sys.exc_info(), "目录错误")
    else:
        # step3: 遍历目录和文件
        lineList = []
        ftp.dir(lineList.append)
        fileList = ftp.nlst()
        index = 0
        for line in lineList:
            if "d" in line.split()[0]:  # 目录类型
                remoteSubPath = remotePath + fileList[index]
                localSubPath = localPath + fileList[index]
                ftpDownload(ftp, remoteSubPath, localSubPath, filterBeforeCB, filterAfterCB)
                ftp.cwd("..")
            else:   # 文件类型
                remoteFilename = remotePath + fileList[index]
                remoteFileSize = int(line.split()[4])
                remoteFileModifyTime = int(ftp.sendcmd("MDTM " + remoteFilename).split()[1])
                localFilename = localPath + fileList[index]
                # 判断是否允许下载
                allowDownload = True
                if hasattr(filterBeforeCB, '__call__'):
                    allowDownload = filterBeforeCB(ftp, remoteFilename, remoteFileSize, remoteFileModifyTime, localFilename)
                if True == allowDownload:
                    try:
                        localFile = open(localFilename, 'wb')       # 打开本地的文件
                        ftp.retrbinary('RETR %s' % remoteFilename, localFile.write) # 用二进制的方式将FTP文件写到本地文件
                        localFile.close()
                    except:
                        print(sys.exc_info(), "文件下载出错", remoteFilename)
                    else:
                        # 判断是否需要删除
                        if hasattr(filterAfterCB, '__call__'):
                            filterAfterCB(ftp, remoteFilename, remoteFileSize, remoteFileModifyTime, localFilename)
            index += 1

""" 过滤策略 """
g_filter_policy = {}

""" 下载前过滤 """
def filterBeforeDownload(ftp, remoteFilename, remoteFileSize, remoteFileModifyTime, localFilename):
    if not g_filter_policy: # 过滤策略为空
        return True
    # step1:过滤大小
    sizeK = math.ceil(remoteFileSize / 1024)
    if sizeK < g_filter_policy["sizeMinKB"] or sizeK > g_filter_policy["sizeMaxKB"]:
        print("文件大小超出范围:", localFilename)
        return False
    # step2:过滤文件后缀名
    extName = os.path.splitext(remoteFilename)[-1][1:]
    if len(g_filter_policy["subFixAllow"]) > 0 and not extName in g_filter_policy["subFixAllow"]:
        print("文件后缀名不匹配:", localFilename)
        return False
    return True

""" 下载后过滤 """
def filterAfterDownload(ftp, remoteFilename, remoteFileSize, remoteFileModifyTime, localFilename):
    if not g_filter_policy: # 过滤策略为空
        return
    # step1:杀毒过滤
    # step2:过滤文件类型(过滤掉非文本文件)
    # if True:
        # return
    # step3:过滤掉非文本文件
    fileContent = ""
    try:
        with open(localFilename, 'r', encoding='utf-8') as fp:
            fileContent = fp.read()
    except:
        print("文件下载成功:", localFilename)
        return
    # step4:过滤白名单
    matchWhite = False
    if not g_filter_policy["contentWhiteList"]:
        matchWhite = True
    else:
        for white in g_filter_policy["contentWhiteList"]:
            if re.match(white["regx"], fileContent):
                matchWhite = True
                break
    if False == matchWhite:
        print("内容不匹配白名单:", localFilename)
        os.remove(localFilename)
        return
    # step5:过滤黑名单
    matchBlack = False
    for black in g_filter_policy["contentBlackList"]:
        if re.match(black["regx"], fileContent):
            matchBlack = True
            break
    if True == matchBlack:
        print("内容触发了黑名单:", localFilename)
        os.remove(localFilename)
        return
    print("文件下载成功:", localFilename)

""" 解析策略文件 """
def parsePolicyFile(filename, ip, port):
    try:
        jsonData = {}
        with open(filename, 'r', encoding='utf-8') as fp:
            jsonContent = fp.read()
            jsonData = json.loads(jsonContent.replace("\\", "\\\\"))
        if not jsonData["data"]:
            return
        if not jsonData["data"]["netFilePolicy"]:
            return
        if not jsonData["data"]["netFilePolicy"]["Policies"]:
            return
        for filePolicy in jsonData["data"]["netFilePolicy"]["Policies"]:
            if ip == filePolicy["serverIp"] and port == filePolicy["serverPort"]:
                return filePolicy["filterPolicy"][0]
    except:
        print(sys.exc_info(), "策略文件", filename, "解析出错")
    return {}

""" 主入口函数 """
def main():
    if 1 == len(sys.argv):
        sys.argv.append("-h")
    parser = argparse.ArgumentParser(description="用于启动FTP客户端",
                                     usage=os.path.basename(__file__) + " [-h] [-i IP] [-p PORT] [-u USERNAME} [-P PASSWORD] [-r REMOTE_PATH]")
    parser.add_argument('-i','--ip',metavar="",type=str,help="指定FTP服务器地址(必填). 例如: 127.0.0.1")
    parser.add_argument('-p','--port',metavar="",type=int,help="指定FTP服务器端口(必填). 例如: 21")
    parser.add_argument('-u',"--username",metavar="",type=str,help="指定用户名(选填). 例如: root. 默认匿名登录")
    parser.add_argument('-P',"--password",metavar="",type=str,help="指定密码(选填). 例如: 123456. 匿名登录时不必填")
    parser.add_argument('-r',"--remote_path",metavar="",type=str,default="/",help="指定要同步的FTP路径(选填). 例如: 根路径/, 子路径/tmp/. 默认值: /")
    parser.add_argument('-l',"--local_path",metavar="",type=str,help="指定本地同步路径(必填). 例如: F:/ftp_tmp")
    parser.add_argument('-f',"--file",metavar="",type=str,help="指定本地策略文件(选填). 例如: F:/ics/policy.json")
    parser.parse_args()
    args = vars(parser.parse_args())
    # step1:必填参数判断
    if not args["ip"]:
        print("--ip 参数缺少")
        return
    if not args["port"]:
        print("--port 参数缺少")
        return
    if not args["local_path"]:
        print("--local_path 参数缺少")
        return
    # step2:加载策略文件
    if args["file"]:
        global g_filter_policy
        g_filter_policy = parsePolicyFile(args["file"], args["ip"], args["port"])
    # step3:FTP流程
    ftp = ftpLogin(args["ip"], args["port"], args["username"], args["password"])
    if not ftp:
        print("FTP 登录失败")
        return
    print("FTP 登录成功")
    print("FTP 文件同步中...")
    ftpDownload(ftp, args["remote_path"], args["local_path"], filterBeforeDownload, filterAfterDownload)
    print("FTP 文件同步结束")
    ftp.quit()
    print("FTP 退出")

if "__main__" == __name__:
    main()
