#!/usr/bin/python2
# -*- coding: UTF-8 -*-
from sdk import network
import argparse
import os
import sys

""" 加载内核模块 """
def loadCoreModule(module):
    lines = os.popen("modprobe " + module).readlines()
    if len(lines) > 0:
        return False
    return True

""" 修改/etc/sysctl.conf """
def modifySysctlConf():
    ip6tables_key = "net.bridge.bridge-nf-call-ip6tables"
    ip6tables_str = "net.bridge.bridge-nf-call-ip6tables = 1"
    ip6tables_flag = False
    iptables_key = "net.bridge.bridge-nf-call-iptables"
    iptables_str = "net.bridge.bridge-nf-call-iptables = 1"
    iptables_flag = False
    arptables_key = "net.bridge.bridge-nf-call-arptables"
    arptables_str = "net.bridge.bridge-nf-call-arptables = 1"
    arptables_flag = False
    ip_forward_key = "net.ipv4.ip_forward"
    ip_forward_str = "net.ipv4.ip_forward = 1"
    ip_forward_flag = False
    ip_nonlocal_bind_key = "net.ipv4.ip_nonlocal_bind"
    ip_nonlocal_bind_str = "net.ipv4.ip_nonlocal_bind = 1"
    ip_nonlocal_bind_flag = False
    default_rp_filter_key = "net.ipv4.conf.default.rp_filter"
    default_rp_filter_str = "net.ipv4.conf.default.rp_filter = 0"
    default_rp_filter_flag = False
    all_rp_filter_key = "net.ipv4.conf.all.rp_filte"
    all_rp_filter_str = "net.ipv4.conf.all.rp_filter = 0"
    all_rp_filter_flag = False
    with open("/etc/sysctl.conf", "r") as p, open("/etc/sysctl.conf.bak", "w") as q:
        for line in p:
            if line.find(ip6tables_key) >= 0:
                if ip6tables_flag:
                    continue
                line = ip6tables_str + "\n"
                ip6tables_flag = True
            elif line.find(iptables_key) >= 0:
                if iptables_flag:
                    continue
                line = iptables_str + "\n"
                iptables_flag = True
            elif line.find(arptables_key) >= 0:
                if arptables_flag:
                    continue
                line = arptables_str + "\n"
                arptables_flag = True
            elif line.find(ip_forward_key) >= 0:
                if ip_forward_flag:
                    continue
                line = ip_forward_str + "\n"
                ip_forward_flag = True
            elif line.find(ip_nonlocal_bind_key) >= 0:
                if ip_nonlocal_bind_flag:
                    continue
                line = ip_nonlocal_bind_str + "\n"
                ip_nonlocal_bind_flag = True
            elif line.find(default_rp_filter_key) >= 0:
                if default_rp_filter_flag:
                    continue
                line = default_rp_filter_str + "\n"
                default_rp_filter_flag = True
            elif line.find(all_rp_filter_key) >= 0:
                if all_rp_filter_flag:
                    continue
                line = all_rp_filter_str + "\n"
                all_rp_filter_flag = True
            q.write(line)
        if False == ip6tables_flag:
            q.write("\n" + ip6tables_str)
        if False == iptables_flag:
            q.write("\n" + iptables_str)
        if False == arptables_flag:
            q.write("\n" + arptables_str)
        if False == ip_forward_flag:
            q.write("\n" + ip_forward_str)
        if False == ip_nonlocal_bind_flag:
            q.write("\n" + ip_nonlocal_bind_str)
        if False == default_rp_filter_flag:
            q.write("\n" + default_rp_filter_str)
        if False == all_rp_filter_flag:
            q.write("\n" + all_rp_filter_str)
    os.popen("mv /etc/sysctl.conf.bak /etc/sysctl.conf")
    os.popen("sysctl -p")

""" 修改网卡配置文件(ubuntu) """
def modifyManagePort(port,address,netmask,gateway):
    buff = "auto "+port+"\n";
    buff += "iface " + port + " inet static\n"
    buff += "address "+address+"\n"
    buff += "network " + network.calcNetAddress(address, netmask) + "\n"
    buff += "netmask " + netmask + "\n"
    buff += "broadcast " + network.calcBroadcastAddress(address) + "\n"
    buff += "gateway " + gateway + "\n"
   
    return buff

def setDefaultGw(defaultGw):
    os.popen("route del -net 0.0.0.0")
    os.popen("route add -net 0.0.0.0 netmask 0.0.0.0 gw "+defaultGw)
    """写入到开机启动脚本里"""
    buff = ""
    with open("/ics/ics_client/ics_start.sh", "r") as p:
        for line in p:
            if line.find("route add -net 0.0.0.0") != -1:
                buff += "route add -net 0.0.0.0 netmask 0.0.0.0 gw " + defaultGw
                break
            buff += line
            print(buff)
    with open("/ics/ics_client/ics_start.sh", "w") as fp:
        fp.write(buff)
    

def modifyBridgePort(netport1, netport2, bridge, address, netmask, gateway, dns):
    
    buff = "auto " + netport1 + "\n"
    buff += "iface " + netport1 + " inet manual\n"
    buff += "\n"
    buff += "auto " + netport2 + "\n"
    buff += "iface " + netport2 + " inet manual\n"
    buff += "\n"
    buff += "auto " + bridge + "\n"
    buff += "iface " + bridge + " inet static\n"
    buff += "address " + address + "\n"
    buff += "network " + network.calcNetAddress(address, netmask) + "\n"
    buff += "netmask " + netmask + "\n"
    buff += "broadcast " + network.calcBroadcastAddress(address) + "\n"
    buff += "gateway " + gateway + "\n"
    buff += "dns-nameservers " + dns + "\n"
    buff += "bridge_ports " + netport1 + "\n"
    buff += "bridge_ports " + netport2 + "\n"
    buff += "bridge_stp off\n"
    buff += "bridge_fd 0\n"
    buff += "bridge_maxwait 0\n"
    buff += "bridge_maxage 12\n"
    
    
    return buff
def writeNetworkCfg(manageCfg,BridgeCfg):
    buff =  "auto lo\n"
    buff += "iface lo inet loopback\n"
    buff += "\n"
    buff += manageCfg
    buff += BridgeCfg
    with open("/etc/network/interfaces", "w") as fp:
        fp.write(buff)

""" 主入口函数 """
def main():
    if 1 == len(sys.argv):
        sys.argv.append("-h")
    parser = argparse.ArgumentParser(description="用于初始化FTP网桥配置",
                                     usage=os.path.basename(__file__) + " [-h] [-dgw defaultGateway][-n1 NETPORT1] [-n2 NETPORT2] [-a ADDRESS] [-m NETMASK] [-g GATEWAY] [-mp MANAGEPORT] [-ma MANAGEPORTADDR] [-mm MANAGEPORTNETMASK] [-mg MANAGEPORTGATEWAY]")
    parser.add_argument('-dgw','--defaultgw',metavar="",type=str,help="指定默认网关")                                 
    parser.add_argument('-n1','--netport1',metavar="",type=str,help="指定网口1(必填). 例如: enp1s0")
    parser.add_argument('-n2','--netport2',metavar="",type=str,help="指定网口2(必填). 例如: enp2s0")
    parser.add_argument('-a','--address',metavar="",type=str,help="指定IP地址(必填). 例如: 192.168.3.107")
    parser.add_argument('-m','--netmask',metavar="",type=str,help="指定子网掩码(必填). 例如: 255.255.255.0")
    parser.add_argument('-g','--gateway',metavar="",type=str,help="指定网关(必填). 例如: 192.168.3.1")
    parser.add_argument('-mp','--manageport',metavar="",type=str,help="指定管理口(必填). 例如: enp3s0")
    parser.add_argument('-ma','--manageaddress',metavar="",type=str,help="指定管理口IP地址(必填). 例如: 192.168.1.107")
    parser.add_argument('-mm','--managenetmask',metavar="",type=str,help="指定管理口子网掩码(必填). 例如: 255.255.255.0")
    parser.add_argument('-mg','--managegateway',metavar="",type=str,help="指定管理口网关(必填). 例如: 192.168.1.1")
    parser.parse_args()
    args = vars(parser.parse_args())
    # step1:必填参数判断
    if not args["netport1"]:
        print("--netport1 参数缺少")
        return
    if not args["netport2"]:
        print("--netport2 参数缺少")
        return
    if not args["address"]:
        print("--address 参数缺少")
        return
    if not args["netmask"]:
        print("--netmask 参数缺少")
        return
    if not args["gateway"]:
        print("--gateway 参数缺少")
        return
    # step1: 加载网桥模块
    if False == loadCoreModule("br_netfilter"):
        print("加载网桥模块 br_netfilter 失败")
        return
    # step2: 加载iptables模块
    if False == loadCoreModule("ip_nat_ftp"):
        print("加载iptables模块 ip_nat_ftp 失败")
        return
    if False == loadCoreModule("ip_conntrack_ftp"):
        print("加载iptables模块 ip_conntrack_ftp 失败")
        return
    # step3: 执行创建br0 
    os.popen("brctl addbr br0")   
    # step4:修改配置
    modifySysctlConf()
    manageCfgBuff = modifyManagePort(args["manageport"],args["manageaddress"],args["managenetmask"],args["managegateway"])
    bridgeBuff = modifyBridgePort(args["netport1"], args["netport2"], "br0", args["address"], args["netmask"], args["gateway"], args["gateway"])
    writeNetworkCfg(manageCfgBuff,bridgeBuff)
    setDefaultGw(args["defaultgw"])
    

if "__main__" == __name__:
    main()
