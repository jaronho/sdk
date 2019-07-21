#!/usr/bin/python2
# -*- coding: UTF-8 -*-
from sdk import kill_process
from sdk import ftp_server
import argparse
import os
import sys

""" 主入口函数 """
def main():
    if 1 == len(sys.argv):
        sys.argv.append("-h")
    parser = argparse.ArgumentParser(description="用于启动FTP服务器",
                                     usage=os.path.basename(__file__) + " [-h] [-d DIR] [-i IP] [-p PORT] [-u USERS]")
    parser.add_argument('-d','--dir',metavar="",type=str,help="指定FTP服务器目录(必填). 例如: E:/")
    parser.add_argument('-i','--ip',metavar="",type=str,default="127.0.0.1",help="指定FTP服务器地址(选填). 默认值: 127.0.0.1")
    parser.add_argument('-p','--port',metavar="",type=int,default=21,help="指定FTP服务器端口(选填). 默认值: 21")
    parser.add_argument('-p1','--pasv_port_begin',metavar="",type=int,default=60000,help="指定FTP服务器被动端口范围起始值(选填). 默认值: 60000")
    parser.add_argument('-p2','--pasv_port_end',metavar="",type=int,default=65535,help="指定FTP服务器被动端口范围结束值(选填). 默认值: 65535")
    parser.add_argument('-u','--users',metavar="",type=str,help="""指定允许访问用户列表,若不设置则任何人都可以访问(选填), 例如:
                                                                   test01:123456,test02:,test03:123123
                                                                   用户名和密码之间由冒号分隔,每个用户之间用英文逗号分割,
                                                                   若密码设置为空,则默认使用用户名作为密码
                                                                """)
    parser.parse_args()
    args = vars(parser.parse_args())
    # step1:必填参数判断
    if not args["dir"]:
        print("--dir 参数缺少")
        return
    # step2:解析参数值
    print("服务器目录: " + args["dir"])
    print("服务器IP: " + args["ip"] + ", 端口: " + str(args["port"]) + ", 被动端口范围: [" + str(args["pasv_port_begin"]) + ", " + str(args["pasv_port_end"]) + ")")
    userList = []
    if args["users"]:
        try:
            userList = args["users"].split(",")
        except:
            print(sys.exc_info(), "用户列表格式错误, 请检查", args["users"])
            return
    users = []
    for user in userList:
        namepassword = user.split(":")
        if 2 == len(namepassword) and len(namepassword[0]) > 0:
            name = namepassword[0]
            password = namepassword[1]
            if 0 == len(password):
                password = name
            users.append({"name":name,"password":password,"permission":"elr"})
            print("允许访问用户: " + name + ", 密码: " + password + ", 权限: elr")
    if 0 == len(users):
        print("允许匿名访问")
    # step3:开启服务器
    pids = kill_process.killByIpPort(args["ip"], args["port"], 1, False)
    if len(pids) > 0:
        print("杀死占用IP和端口的进程:", pids)
    ftp_server.ftpStart(dir=args["dir"], ip=args["ip"], pasvPortBegin=args["pasv_port_begin"], pasvPortEnd=args["pasv_port_end"], port=args["port"], users=users)

if "__main__" == __name__:
    main()
