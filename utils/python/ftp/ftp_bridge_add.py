#!/usr/bin/python2
# -*- coding: UTF-8 -*-
import argparse
import os
import sys

""" 添加iptables策略 """
def addIptablesPolicy(ip, port, localIp, localPort, pasvPort):
    os.popen("iptables -t nat -A PREROUTING -d " + ip + " -p tcp -m tcp --dport " + str(port) + " -j DNAT --to-destination " + localIp + ":" + str(localPort))
    os.popen("iptables -t nat -A PREROUTING -d " + ip + " -p tcp -m tcp --dport " + str(pasvPort) + " -j DNAT --to-destination " + localIp + ":" + str(pasvPort))

""" 主入口函数 """
def main():
    if 1 == len(sys.argv):
        sys.argv.append("-h")
    parser = argparse.ArgumentParser(description="用于添加FTP桥接策略",
                                     usage=os.path.basename(__file__) + " [-i IP] [-p PORT] [-li LOCAL_IP] [-lp LOCAL_PORT] [-pp PASV_PORT]")
    parser.add_argument('-i','--ip',metavar="",type=str,help="指定远端FTP服务器IP地址(必填). 例如: 192.168.3.254")
    parser.add_argument('-p','--port',metavar="",type=int,help="指定远端FTP服务器端口(必填). 例如: 2124")
    parser.add_argument('-li','--local_ip',metavar="",type=str,help="指定本机FTP服务器IP地址(必填). 例如: 192.168.3.107")
    parser.add_argument('-lp','--local_port',metavar="",type=int,help="指定本机FTP服务器端口(必填). 例如: 2126")
    parser.add_argument('-pp','--pasv_port',metavar="",type=int,help="指定本机FTP服务器被动端口(必填). 例如: 60000")
    parser.parse_args()
    args = vars(parser.parse_args())
    # step1:必填参数判断
    if not args["ip"]:
        print("--ip 参数缺少")
        return
    if not args["port"]:
        print("--port 参数缺少")
        return
    if not args["local_ip"]:
        print("--local_ip 参数缺少")
        return
    if not args["local_port"]:
        print("--local_port 参数缺少")
        return
    if not args["pasv_port"]:
        print("--pasv_port 参数缺少")
        return
    addIptablesPolicy(args["ip"], args["port"], args["local_ip"], args["local_port"], args["pasv_port"])

if "__main__" == __name__:
    main()
