#!/usr/bin/python2
# -*- coding: UTF-8 -*-
from sdk import filetype
from sdk import ftp_client
from sdk import kill_process
import argparse
import datetime
import json
import math
import os
import platform
import re
import sys
import syslog

""" 远程目录 """
g_remote_path = ""
""" 本地目录 """
g_local_path = ""
""" 校验文件名 """
g_checksum_filename = ""
""" 校验文件列表 """
g_checksum_list = []
""" 缓存天数 """
g_cache_days = 0
""" 过滤策略列表 """
g_filter_policys = []

""" 解析校验文件 """
def parseChecksumFile(filename):
    try:
        jsonData = []
        with open(filename, 'r', encoding='utf-8') as fp:
            jsonContent = fp.read()
            if len(jsonContent) > 0:
                jsonData = json.loads(jsonContent.replace("\\", "\\\\"))
        return jsonData
    except:
        print(sys.exc_info(), "校验文件", filename, "解析出错")
    return []

""" 保存校验文件 """
def saveChecksumFile(filename, checksumList):
    try:
        jsonContent = json.dumps(checksumList, ensure_ascii=False, indent="    ")
        with open(filename, 'w+', encoding='utf-8') as fp:
            fp.write(jsonContent)
    except:
        print(sys.exc_info(), "校验文件保存出错")

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
                return [filePolicy["cacheDays"], filePolicy["filterPolicy"]]
    except:
        print(sys.exc_info(), "策略文件", filename, "解析出错")
    return [0, []]

""" 删除废弃文件 """
def removeWasteFile(localFilename):
    for i in range(len(g_checksum_list)):
        if localFilename == g_checksum_list[i]["name"]:
            g_checksum_list.pop(i)
            saveChecksumFile(g_checksum_filename, g_checksum_list)
            os.remove(localFilename)
            break

""" 根据后缀名获取策略 """
def getPolicyBySuffix(remoteFilename):
    for policy in g_filter_policys:
        # 过滤文件后缀名
        extName = os.path.splitext(remoteFilename)[-1][1:]
        if len(policy["subFixAllow"]) > 0 and extName in policy["subFixAllow"]:
            return policy
    return None

""" 检测本地文件是否合法 """
def checkLocalFile(remoteFilename, localFilename, policy):
    if not policy:
        return True
    # 过滤文本文件内容
    isTextFile = False
    if "Linux" == platform.system():
        if "text" in os.popen("file " + localFilename).readlines()[0].split(":")[1]:
            isTextFile = True
    if True == isTextFile:
        fileContent = ""
        with open(localFilename, 'rt', encoding='utf-8') as fp:
            fileContent = fp.read()
        # 过滤白名单
        matchWhite = False
        if not policy["contentWhiteList"]:
            matchWhite = True
        else:
            for white in policy["contentWhiteList"]:
                if 0 == len(white["regx"]) or re.match(white["regx"], fileContent):
                    matchWhite = True
                    break
        if False == matchWhite:
            print("内容不匹配白名单:", remoteFilename)
            syslog.syslog("内容不匹配白名单: " + remoteFilename)
            return False
        # 过滤黑名单
        matchBlack = False
        for black in policy["contentBlackList"]:
            if len(black["regx"]) > 0 and re.match(black["regx"], fileContent):
                matchBlack = True
                break
        if True == matchBlack:
            print("内容触发了黑名单:", remoteFilename)
            syslog.syslog("内容触发了黑名单: " + remoteFilename)
            return False
    else:
        # 过滤文件类型
        isFileTypeAllow = False
        with open(localFilename, 'rb') as fp:
            for fileAllow in policy["fileTypeAllow"]:
                offset = 0
                if fileAllow.has_key("offset"):
                    offset = fileAllow["offset"]
                if filetype.isFileHeadWith(fp, fileAllow["typeBytes"], offset):
                    isFileTypeAllow = True
                    break
        if False == isFileTypeAllow:
            print("文件类型不被允许:", remoteFilename)
            syslog.syslog("文件类型不被允许: " + remoteFilename)
            return False
    return True

""" 文件列表搜索完毕 """
def fileListSearchOver(list):
    # 删除不在服务端的本地文件
    for checksum in g_checksum_list:
        relativePath = checksum["name"][len(g_local_path):]
        isRemoteExist = False
        for item in list:
            if not item["isdir"] and relativePath == (item["path"] + item["name"])[len(g_remote_path):]:
                isRemoteExist = True
                break
        if not isRemoteExist:
            print("文件在远端不存在:", checksum["name"])
            g_checksum_list.remove(checksum)
            os.remove(checksum["name"])
    saveChecksumFile(g_checksum_filename, g_checksum_list)

""" 下载前过滤 """
def filterBeforeDownload(ftp, remoteFilename, remoteFileSize, remoteFileModifyTime, localFilename):
    # step1:过滤文件缓存时间(当前时间-文件修改时间)
    rYear = int(str(remoteFileModifyTime)[0:4])
    rMonth = int(str(remoteFileModifyTime)[4:6])
    rDay = int(str(remoteFileModifyTime)[6:8])
    dayDiff = (datetime.datetime.now() - datetime.datetime(rYear, rMonth, rDay)).days
    if g_cache_days > 0 and dayDiff > g_cache_days:
        print("文件超过缓存时间:", remoteFilename)
        syslog.syslog("文件超过缓存时间: " + remoteFilename)
        removeWasteFile(localFilename)
        return False
    # step2:过滤策略
    policy = None
    if len(g_filter_policys) > 0:
        policy = getPolicyBySuffix(remoteFilename)
        # 过滤文件后缀名
        if not policy:
            print("文件后缀名不匹配:", remoteFilename)
            syslog.syslog("文件后缀名不匹配: " + remoteFilename)
            removeWasteFile(localFilename)
            return False
        # 过滤文件大小
        sizeK = math.ceil(remoteFileSize / 1024)
        if sizeK < policy["sizeMinKB"] or sizeK > policy["sizeMaxKB"]:
            print("文件大小超出范围:", remoteFilename)
            syslog.syslog("文件大小超出范围: " + remoteFilename)
            return False
    # step3:过滤相同文件(根据文件大小和修改时间是否一致判断)
    for i in range(len(g_checksum_list)):
        checksum = g_checksum_list[i]
        if localFilename == checksum["name"]:
            if remoteFileSize == checksum["size"] and remoteFileModifyTime == checksum["mtime"]:
                if checkLocalFile(remoteFilename, localFilename, policy):
                    return False
            g_checksum_list.pop(i)
            saveChecksumFile(g_checksum_filename, g_checksum_list)
            os.remove(localFilename)
            break
    return True

""" 下载后过滤 """
def filterAfterDownload(ftp, remoteFilename, remoteFileSize, remoteFileModifyTime, localFilename):
    # step1:杀毒过滤
    results = os.popen("python antivirus.py -p " + localFilename).read()
    if len(results) > 0:
        print("文件发现病毒内容:", remoteFilename)
        syslog.syslog("文件发现病毒内容: " + remoteFilename)
        os.remove(localFilename)
        return
    # step2:过滤策略
    policy = getPolicyBySuffix(remoteFilename)
    if not checkLocalFile(remoteFilename, localFilename, policy):
        os.remove(localFilename)
        return
    # step3:更新校验文件
    g_checksum_list.append({"name":localFilename, "size":remoteFileSize, "mtime":remoteFileModifyTime})
    saveChecksumFile(g_checksum_filename, g_checksum_list)
    print("文件下载成功:", localFilename)

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
    elif False == os.path.exists(args["local_path"]):
        os.makedirs(args["local_path"])
    # step2:如果已经有进程正在同步,则本进程退出
    clientPids = kill_process.findPids(args["ip"], args["port"], 2)
    if len(clientPids) > 0:
        print("当前有进程正在FTP同步中")
        return
    syslog.openlog("[ics_client:ftp_client_start.py] ", syslog.LOG_PID)
    # step3:规范化目录格式
    global g_remote_path
    g_remote_path = args["remote_path"]
    if False == g_remote_path.endswith("/"):
        g_remote_path += "/"
    global g_local_path
    g_local_path = args["local_path"]
    if False == g_local_path.endswith("/"):
        g_local_path += "/"
    # step4:加载校验文件
    global g_checksum_filename
    g_checksum_filename = g_local_path + "__checksum__.json"
    global g_checksum_list
    g_checksum_list = parseChecksumFile(g_checksum_filename)
    # step5:加载策略文件
    if args["file"]:
        policyInfo = parsePolicyFile(args["file"], args["ip"], args["port"])
        global g_cache_days
        g_cache_days = policyInfo[0]
        global g_filter_policys
        g_filter_policys = policyInfo[1]
    # step6:FTP流程
    ftp = ftp_client.ftpLogin(args["ip"], args["port"], args["username"], args["password"])
    if not ftp:
        print("FTP 登录失败")
        syslog.closelog()
        return
    print("FTP 登录成功")
    print("FTP 文件同步中...")
    ftp_client.ftpDownload(ftp, g_remote_path, g_local_path, fileListSearchOver, filterBeforeDownload, filterAfterDownload)
    print("FTP 文件同步结束")
    ftp.quit()
    print("FTP 退出")
    syslog.closelog()

if "__main__" == __name__:
    main()
