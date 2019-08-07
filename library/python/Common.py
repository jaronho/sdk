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
