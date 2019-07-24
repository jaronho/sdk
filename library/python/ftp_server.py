#!/usr/bin/python2
# -*- coding: UTF-8 -*-
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import ThreadedFTPServer
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

"""
 * Brief:   开启FTP服务器
 * Param:   anoDir - 匿名用户允许访问的目录,例如: /home/
 *          anoPerm - 匿名用户允许访问的权限,例如: elr
 *          ip - 地址(选填),例如: 127.0.0.1
 *          port - 端口(选填),例如: 21
 *          pasvPortBegin - 被动模式开放的起始端口(选填),例如: 60000
 *          pasvPortEnd - 被动模式开放的截止端口(选填),例如: 65535
 *          users - 用户列表(选填),默认没有, 例如: [{"name":"test1","pwd":"123456","dir":"/home/","perm":"elr"},{"name":"test2","pwd":"123456","dir":"/home/","perm":"elr"}]
 *          maxCons - 最大连接数(选填),例如: 512
 *          maxConsPerIp - 每个IP最大连接数(选填),0:没有限制,默认为0
 * Return:  空
"""
def ftpStart(anoDir="", anoPerm="elr", ip="127.0.0.1", port=21, pasvPortBegin=60000, pasvPortEnd=65535, users=[], maxCons=512, maxConsPerIp=0):
    try:
        # step1:实例化用户授权管理
        authorizer = DummyAuthorizer()
        # step2:添加匿名用户,任何人都可以访问
        if len(anoDir) > 0:
            authorizer.add_anonymous(anoDir, perm=anoPerm)
        # step3:添加非匿名用户名,密码,指定目录,权限
        for user in users:
            authorizer.add_user(user["name"], user["pwd"], user["dir"], perm=user["perm"])
        # step4:实例化FTPHandler
        handler = FTPHandler
        handler.authorizer = authorizer
        handler.passive_ports = range(pasvPortBegin, pasvPortEnd)
        # step5:配置服务器
        server = ThreadedFTPServer((ip, port), handler)
        server.max_cons = maxCons
        server.max_cons_per_ip = maxConsPerIp
        # step6:开启服务器
        server.serve_forever()
    except:
        print(sys.exc_info())
