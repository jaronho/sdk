# -*- coding: utf-8 -*-
import os
import platform
import re
import sys

"""
 * Brief:   获取父级路径
 * Param:   path - 路径,例如: /home/dev/workspace,得到:/home/dev/
 * Return:  字符串
"""
def getParentPath(path):
    path = path.replace('\\', '/')
    if path.endswith('/'):
        path = path[0:-1]
    p = path.rfind('/')
    return path[0:p + 1]

"""
 * Brief:   删除文件/目录
 * Param:   name - 文件名,目录名,例如: /home/dev/aa.txt 或 /home/dev/test
 * Return:  空
"""
def remove(name):
    # 判断文件是否存在
    if not os.path.exists(name):
        return
    # 是普通文件
    if os.path.isfile(name):
        os.remove(name)
        return
    # 是目录文件
    dirs = os.listdir(name)
    for f in dirs:
        itemName = os.path.join(name, f)    # 拼接文件名
        if os.path.isfile(itemName):    # 普通文件
            os.remove(itemName)
        else:   # 目录文件,递归处理
            remove(itemName)
    # 删除空目录
    os.rmdir(name)
