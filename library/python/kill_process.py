#!/usr/bin/python2
# -*- coding: UTF-8 -*-
import argparse
import os
import platform
import sys
import time

"""
 * Brief:   根据IP和端口查找关联的进程ID
 * Param:   ip - 地址,例如: 127.0.0.1
 *          port - 端口,例如: 21
 *          target - 指定目标(选填): 1-占用,2-连接,默认查找占用目标
 * Return:  进程ID列表
"""
def findPids(ip, port, target=1):
    if 1 != target and 2 != target:
        return None
    matchStr = ip + ":" + str(port)
    pids = []
    if "Windows" == platform.system():
        lines = os.popen("netstat -ano | findstr " + matchStr).readlines()
        for line in lines:
            info = line.strip().split()
            if (5 == len(info)) and \
               ((1 == target and "LISTENING" == info[3] and matchStr == info[1]) or \
                (2 == target and "ESTABLISHED" == info[3] and matchStr == info[2])):
                pids.append(info[4])
    elif "Linux" == platform.system():
        lines = os.popen("netstat -anp | grep " + matchStr).readlines()
        for line in lines:
            info = line.strip().split()
            if (7 == len(info)) and \
               ((1 == target and "LISTEN" == info[5] and matchStr == info[3]) or \
                (2 == target and "ESTABLISHED" == info[5] and matchStr == info[4])):
                pids.append(info[6].split('/')[0])
    return pids

"""
 * Brief:   杀死进程ID
 * Param:   pids - 进程ID列表
 * Return:  空
"""
def killPids(pids):
    if "Windows" == platform.system():
        for pid in pids:
            os.popen('taskkill -f -pid {0}'.format(int(pid)))
        time.sleep(1)
    elif "Linux" == platform.system():
        for pid in pids:
            os.popen('kill -9 {0}'.format(int(pid)))

"""
 * Brief:   杀死进程(根据IP地址和端口)
 * Param:   ip - 地址,例如: 127.0.0.1
 *          port - 端口,例如: 21
 *          target - 指定目标(选填): 1-占用,2-连接,默认查找占用目标
 *          doPrint - 是否打印消息,默认打印
 * Return:  被杀死的进程ID列表
"""
def killByIpPort(ip, port, target=1, doPrint=True):
    pids = findPids(ip, port, target)
    if 0 == len(pids):
        if doPrint:
            print("没有进程可杀")
        return pids
    elif doPrint:
        print("准备杀死进程: " + str(pids))
    killPids(pids)
    return pids

""" 主入口函数 """
def main():
    if 1 == len(sys.argv):
        sys.argv.append("-h")
    parser = argparse.ArgumentParser(description="用于杀死(占用,连接)指定IP地址和端口的进程",
                                     usage=os.path.basename(__file__) + " [-h] [-i IP] [-p PORT] [-t TARGET]")
    parser.add_argument('-i','--ip',metavar="",type=str,help="指定IP地址(必填), 例如: 127.0.0.1")
    parser.add_argument('-p','--port',metavar="",type=int,help="指定端口(必填), 例如: 21")
    parser.add_argument('-t','--target',metavar="",type=int,default=1,help="指定杀死目标(选填): 1-占用,2-连接. 默认值: 1")
    parser.parse_args()
    args = vars(parser.parse_args())
    # step1:必填参数判断
    if not args["ip"]:
        print("--ip 参数缺少")
        return
    if not args["port"]:
        print("--port 参数缺少")
        return
    if 1 != args["target"] and 2 != args["target"]:
        print("--target 参数取值范围: [1, 2]")
        return
    # step2:杀死进程
    killByIpPort(args["ip"], args["port"], args["target"])

if "__main__" == __name__:
    main()

