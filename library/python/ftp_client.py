# -*- coding: utf-8 -*-
from ftplib import FTP
import os
import sys

"""
 * Brief:   FTP登录
 * Param:   host - 地址,例如: 127.0.0.1
 *          port - 端口,例如: 21
 *          user - 用户名(选填),默认匿名登录
 *          password - 密码(选填),匿名登录时可不填
 *          pasv - 是否被动模式,True:被动模式,False:主动模式
 * Return:  FTP实例对象
"""
def ftpLogin(host, port, user="anonymous", password="", pasv=True):
    try:
        ftp = FTP()
        ftp.encoding = "utf-8"
        ftp.set_pasv(pasv)      # True.被动模式,False.主动模式
        ftp.connect(host, port)
        ftp.login(user, password)
        return ftp
    except:
        print("Exception: ftp_client.ftpLogin =>\n           " + str(sys.exc_info()))
    return None

"""
 * Brief:   递归搜索FTP指定目录
 * Param:   ftp - FTP实例对象
 *          path - 指定要搜索的服务器目录(选填),默认服务器根目录
 *          list - 搜索结果列表
 * Return:  服务器目录信息
"""
def ftpList(ftp, path="/", list=[]):
    dirInfo = {
        "size": 0,          # 总大小
        "file": 0,          # 文件数
        "folder": 0         # 目录数
    }
    try:
        if False == path.endswith("/"):
            path += "/"
        ftp.cwd(path)
        lineList = []
        ftp.dir(lineList.append)
        for line in lineList:
            # 解析名称
            index = 0
            num = 0
            isPreSpace = False
            for i in range(len(line)):
                if ' ' == line[i]:
                    isPreSpace = True
                else:
                    if 0 == num or True == isPreSpace:
                        isPreSpace = False
                        num = num + 1
                        if 9 == num:    # 第9个非空格字符串
                            index = i
                            break
            name = line[index:]
            if "." == name or ".." == name:
                continue
            # 类型判断
            if line.startswith("d"):  # 目录类型
                subPath = path + name + "/"
                item = {
                    "type": 0,                      # 类型:0.目录,1.文件,2.软链接,3.其他
                    "path": path,                   # 父目录(绝对路径)
                    "name": name,                   # 目录名称
                    "size": 0,                      # 目录大小
                    "file": 0,                      # 文件数(递归)
                    "folder": 0                     # 目录数(递归)
                }
                list.append(item)
                subDirInfo = ftpList(ftp, subPath, list)
                ftp.cwd("..")
                item["size"] = subDirInfo["size"]
                item["file"] = subDirInfo["file"]
                item["folder"] = subDirInfo["folder"]
                dirInfo["size"] += subDirInfo["size"]
                dirInfo["file"] += subDirInfo["file"]
                dirInfo["folder"] += 1 + subDirInfo["folder"]
            elif line.startswith("-"):   # 文件类型
                ftp.voidcmd("TYPE I")
                filename = path + name
                fileSize = int(float(ftp.sendcmd("SIZE " + filename).split()[1]))
                fileModifyTime = int(float(ftp.sendcmd("MDTM " + filename).split()[1]))
                item = {
                    "type": 1,                      # 类型:0.目录,1.文件,2.软链接,3.其他
                    "path": path,                   # 父目录(绝对路径)
                    "name": name,                   # 文件名称
                    "size": fileSize,               # 文件大小
                    "mtime": fileModifyTime         # 文件修改时间
                }
                list.append(item)
                dirInfo["size"] += fileSize
                dirInfo["file"] += 1
            elif line.startswith("l"):  # 软链接类型
                print("symlink: " + line)
            else:   # 其他类型
                print("other: " + line)
    except:
        print("Exception: ftp_client.ftpList =>\n           " + str(sys.exc_info()))
    return dirInfo

"""
 * Brief:   从FTP服务器指定目录下载文件到本地指定目录
 * Param:   ftp - FTP实例对象
 *          remotePath - 指定要下载的服务器目录,例如:"/"
 *          localPath - 本地目录,例如:"E:/ftp_tmp"
 *          listOverCB - 文件列表搜索完毕回调函数
 *          filterDirectoryCB - 目录过滤回调函数
 *          filterFileBeforeCB - 下载前文件过滤回调函数
 *          filterFileAfterCB - 下载后文件过滤回调函数
 * Return:  空
"""
def ftpDownload(ftp, remotePath, localPath, listOverCB=None, filterDirectoryCB=None, filterFileBeforeCB=None, filterFileAfterCB=None):
    try:
        if False == remotePath.endswith("/"):
            remotePath += "/"
        if False == localPath.endswith("/"):
            localPath += "/"
        if False == os.path.exists(localPath):
            os.makedirs(localPath)
        # 遍历FTP路径文件
        list = []
        ftpList(ftp, remotePath, list)
        if hasattr(listOverCB, '__call__'):
            listOverCB(list)
        # 遍历判断已搜索到的文件
        notAllowDirList = []
        for item in list:
            remoteItemName = item["path"] + item["name"]
            localItemName = localPath + item["path"][len(remotePath):] + item["name"]
            # 判断所在目录是否被过滤
            if item["path"] in notAllowDirList:
                continue
            # 判断是否允许创建目录
            if 0 == item["type"]:
                allowMakeDir = True
                if hasattr(filterDirectoryCB, '__call__'):
                    allowMakeDir = filterDirectoryCB(ftp, remoteItemName, item["size"], item["file"], item["folder"], localItemName)
                if True == allowMakeDir:
                    if False == os.path.exists(localItemName):
                        os.makedirs(localItemName)
                else:
                    if False == remoteItemName.endswith("/"):
                        remoteItemName += "/"
                    notAllowDirList.append(remoteItemName)
                continue
            # 判断是否允许下载
            allowDownload = True
            if hasattr(filterFileBeforeCB, '__call__'):
                allowDownload = filterFileBeforeCB(ftp, remoteItemName, item["size"], item["mtime"], localItemName)
            if not allowDownload:
                continue
            try:
                localFile = open(localItemName, 'wb')       # 打开本地的文件
                ftp.retrbinary('RETR %s' % remoteItemName, localFile.write) # 用二进制的方式将FTP文件写到本地文件
                localFile.close()
            except:
                print("Exception: ftp_client.ftpDownload[open] =>\n           " + str(sys.exc_info()) + "\n           " + remoteItemName)
            else:
                # 判断是否需要删除
                if hasattr(filterFileAfterCB, '__call__'):
                    filterFileAfterCB(ftp, remoteItemName, item["size"], item["mtime"], localItemName)
    except:
        print("Exception: ftp_client.ftpDownload =>\n           " + str(sys.exc_info()))
