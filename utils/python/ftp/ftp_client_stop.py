#!/usr/bin/python2
# -*- coding: UTF-8 -*-
from sdk import kill_process
import argparse
import os
import sys

""" 主入口函数 """
def main():
    if 1 == len(sys.argv):
        sys.argv.append("-h")
    parser = argparse.ArgumentParser(description="用于停止FTP客户端",
                                     usage=os.path.basename(__file__) + " [-h] [-i IP] [-p PORT]")
    parser.add_argument('-i','--ip',metavar="",type=str,default="127.0.0.1",help="指定FTP服务器地址(选填). 默认值: 127.0.0.1")
    parser.add_argument('-p','--port',metavar="",type=int,default=21,help="指定FTP服务器端口(选填). 默认值: 21")
    parser.parse_args()
    args = vars(parser.parse_args())
    # 杀死客户端进程
    pids = kill_process.killByIpPort(args["ip"], args["port"], 2, False)
    if 0 == len(pids):
        print("当前主机没有进程连接IP: " + args["ip"] + ", 端口: " + str(args["port"]))
    else:
        print("杀死连接IP: " + args["ip"] + ", 端口: " + str(args["port"]) + " 的进程: " + str(pids))

if "__main__" == __name__:
    main()
