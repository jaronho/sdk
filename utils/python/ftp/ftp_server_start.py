#!/usr/bin/python2
# -*- coding: UTF-8 -*-
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import ThreadedFTPServer
from sdk import kill_process
import argparse
import os
import sys

"""
读取权限:
'e' = 更改目录(CWD,CDUP命令)
'l' = 列表文件(LIST,NLST,STAT,MLSD,MLST,SIZE命令)
'r' = 从服务器检索文件(RETR命令)
写入权限:
'a' = 将数据追加到现有文件(APPE命令)
'd' = 删除文件或目录(DELE,RMD命令)
'f' = 重命名文件或目录(RNFR,RNTO命令)
'm' = 创建目录(MKD命令)
'w' = 将文件存储到服务器(STOR,STOU命令)
'M' = 更改文件模式/权限(SITE,CHMOD命令)
'T' = 更改文件修改时间(SITE,MFMT命令)
"""

""" 开启FTP服务器 """
def startServer(dir, ip="127.0.0.1", port=21, users=[], maxCons=256, maxConsPerIp=5):
    # step1:实例化用户授权管理
    authorizer = DummyAuthorizer()
    if 0 == len(users):  # 添加:匿名用户,任何人都可以访问(只读权限)
        authorizer.add_anonymous(dir, perm="elr")
    else:   # 添加:用户名,密码,指定目录,权限
        for user in users:
            authorizer.add_user(user["name"], user["password"], dir, perm=user["permission"])
    # step2:实例化FTPHandler
    handler = FTPHandler
    handler.authorizer = authorizer
    # step3:配置服务器
    server = ThreadedFTPServer((ip, port), handler)
    server.max_cons = maxCons
    server.max_cons_per_ip = maxConsPerIp
    # step4:开启服务器
    server.serve_forever()

""" 主入口函数 """
def main():
    if 1 == len(sys.argv):
        sys.argv.append("-h")
    parser = argparse.ArgumentParser(description="用于启动FTP服务器",
                                     usage=os.path.basename(__file__) + " [-h] [-d DIR] [-i IP] [-p PORT] [-u USERS]")
    parser.add_argument('-d','--dir',metavar="",type=str,help="指定FTP服务器目录(必填). 例如: E:/")
    parser.add_argument('-i','--ip',metavar="",type=str,default="127.0.0.1",help="指定FTP服务器地址(选填). 默认值: 127.0.0.1")
    parser.add_argument('-p','--port',metavar="",type=int,default=21,help="指定FTP服务器端口(选填). 默认值: 21")
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
    print("服务器IP: " + args["ip"] + ", 端口: " + str(args["port"]))
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
    startServer(dir=args["dir"], ip=args["ip"], port=args["port"], users=users)

if "__main__" == __name__:
    main()
