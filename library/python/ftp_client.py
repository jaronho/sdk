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
        fileList = ftp.nlst()
        index = 0
        for line in lineList:
            if line.startswith("d"):  # 目录类型
                subPath = path + fileList[index] + "/"
                item = {
                    "isdir": 1,                     # 是否目录
                    "path": path,                   # 父目录(绝对路径)
                    "name": fileList[index],        # 目录名称
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
            else:   # 文件类型
                ftp.voidcmd("TYPE I")
                filename = path + fileList[index]
                fileSize = int(ftp.sendcmd("SIZE " + filename).split()[1])
                fileModifyTime = int(ftp.sendcmd("MDTM " + filename).split()[1])
                item = {
                    "isdir": 0,                     # 是否目录
                    "path": path,                   # 父目录(绝对路径)
                    "name": fileList[index],        # 文件名称
                    "size": fileSize,               # 文件大小
                    "mtime": fileModifyTime         # 文件修改时间
                }
                list.append(item)
                dirInfo["size"] += fileSize
                dirInfo["file"] += 1
            index += 1
    except:
        print("Exception: ftp_client.ftpList =>\n           " + str(sys.exc_info()))
    return dirInfo

"""
 * Brief:   从FTP服务器指定目录下载文件到本地指定目录
 * Param:   ftp - FTP实例对象
 *          remotePath - 指定要下载的服务器目录,例如:"/"
 *          localPath - 本地目录,例如:"E:/ftp_tmp"
 *          listOverCB - 文件列表搜索完毕回调函数
 *          filterBeforeCB - 下载前过滤回调函数
 *          filterAfterCB - 下载后过滤回调函数
 * Return:  空
"""
def ftpDownload(ftp, remotePath, localPath, listOverCB=None, filterBeforeCB=None, filterAfterCB=None):
    try:
        if False == remotePath.endswith("/"):
            remotePath += "/"
        if False == localPath.endswith("/"):
            localPath += "/"
        if False == os.path.exists(localPath):
            os.makedirs(localPath)
        list = []
        ftpList(ftp, remotePath, list)
        if hasattr(listOverCB, '__call__'):
            listOverCB(list)
        for item in list:
            localItemName = localPath + item["path"][len(remotePath):] + item["name"]
            if item["isdir"]:
                if False == os.path.exists(localItemName):
                    os.makedirs(localItemName)
                continue
            remoteFilename = item["path"] + item["name"]
            # 判断是否允许下载
            allowDownload = True
            if hasattr(filterBeforeCB, '__call__'):
                allowDownload = filterBeforeCB(ftp, remoteFilename, item["size"], item["mtime"], localItemName)
            if not allowDownload:
                continue
            try:
                localFile = open(localItemName, 'wb')       # 打开本地的文件
                ftp.retrbinary('RETR %s' % remoteFilename, localFile.write) # 用二进制的方式将FTP文件写到本地文件
                localFile.close()
            except:
                print("Exception: ftp_client.ftpDownload[open] =>\n           " + str(sys.exc_info()) + "\n           " + remoteFilename)
            else:
                # 判断是否需要删除
                if hasattr(filterAfterCB, '__call__'):
                    filterAfterCB(ftp, remoteFilename, item["size"], item["mtime"], localItemName)
    except:
        print("Exception: ftp_client.ftpDownload =>\n           " + str(sys.exc_info()))
