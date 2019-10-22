# -*- coding: utf-8 -*-
from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import MultiprocessFTPServer, ThreadedFTPServer
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

# class FTPHandlerCB(FTPHandler):
    # def on_connect(self):
        # """Called when client connects, *before* sending the initial
        # 220 reply.
        # """

    # def on_disconnect(self):
        # """Called when connection is closed."""

    # def on_login(self, username):
        # """Called on user login."""

    # def on_login_failed(self, username, password):
        # """Called on failed login attempt.
        # At this point client might have already been disconnected if it
        # failed too many times.
        # """

    # def on_logout(self, username):
        # """Called when user "cleanly" logs out due to QUIT or USER
        # issued twice (re-login). This is not called if the connection
        # is simply closed by client.
        # """

    # def on_file_sent(self, file):
        # """Called every time a file has been succesfully sent.
        # "file" is the absolute name of the file just being sent.
        # """

    # def on_file_received(self, file):
        # """Called every time a file has been succesfully received.
        # "file" is the absolute name of the file just being received.
        # """

    # def on_incomplete_file_sent(self, file):
        # """Called every time a file has not been entirely sent.
        # (e.g. ABOR during transfer or client disconnected).
        # "file" is the absolute name of that file.
        # """

    # def on_incomplete_file_received(self, file):
        # """Called every time a file has not been entirely received
        # (e.g. ABOR during transfer or client disconnected).
        # "file" is the absolute name of that file.
        # """

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
 *          handler - 处理类(选填),参考:http://pydoc.net/pyftpdlib/1.3.0/pyftpdlib.handlers
 *          isMultiProcess - 是否支持多进程(选填),默认是
 * Return:  空
"""

def ftpStart(anoDir="", anoPerm="elr", ip="127.0.0.1", port=21, pasvPortBegin=60000, pasvPortEnd=65535, users=[], maxCons=512, maxConsPerIp=0, handler=None, isMultiProcess=True):
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
        if not handler:
            handler = FTPHandler
        handler.authorizer = authorizer
        handler.passive_ports = range(pasvPortBegin, pasvPortEnd)
        # step5:配置服务器
        if isMultiProcess:
            server = MultiprocessFTPServer((ip, port), handler)
        else:
            server = ThreadedFTPServer((ip, port), handler)
        server.max_cons = maxCons
        server.max_cons_per_ip = maxConsPerIp
        # step6:开启服务器
        server.serve_forever()
    except:
        print("Exception: ftp_server.ftpStart =>\n           " + str(sys.exc_info()))
