# -*- coding: utf-8 -*-
import os
import re

"""
 * Brief:   是否IPv4地址
 * Param:   ip - 地址,例如: 192.168.3.107
 * Return:  布尔值
"""
def isIPv4(ip):
    if not isinstance(ip, str):
        return False
    compileIp = re.compile('^(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|[1-9])\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)$')
    if compileIp.match(ip):
        return True
    return False

"""
 * Brief:   根据IP和子网掩码计算网络地址
 * Param:   ip - 地址,例如: 192.168.3.107
 *          netmask - 子网掩码,例如: 255.255.255.0
 * Return:  网络地址 = ip & netmask
"""
def calcNetAddress(ip, netmask):
    if False == isIPv4(ip) or False == isIPv4(netmask):
        return None
    ipArr = ip.split('.')
    maskArr = netmask.split('.')
    addressArr = list(map(lambda x: str(int(x[0]) & int(x[1])), list(zip(ipArr, maskArr))))
    return '.'.join(addressArr)

"""
 * Brief:   根据IP和子网掩码计算主机地址
 * Param:   ip - 地址,例如: 192.168.3.107
 *          netmask - 子网掩码,例如: 255.255.255.0
 * Return:  主机地址 = ip & (~netmask)
"""
def calcHostAddress(ip, netmask):
    if False == isIPv4(ip) or False == isIPv4(netmask):
        return None
    ipArr = ip.split('.')
    maskArr = netmask.split('.')
    reverseMaskArr = list(map(lambda x: abs(255 - int(x)), maskArr))
    addressArr = list(map(lambda x: str(int(x[0]) & int(x[1])), list(zip(ipArr, reverseMaskArr))))
    return '.'.join(addressArr)

"""
 * Brief:   根据IP计算广播地址
 * Param:   ip - 地址,例如: 192.168.3.107
 * Return:  广播地址 = ip[0].ip[1].ip[2].255
"""
def calcBroadcastAddress(ip):
    if False == isIPv4(ip):
        return None
    ipArr = ip.split('.')
    ipArr[3] = "255"
    return '.'.join(ipArr)
