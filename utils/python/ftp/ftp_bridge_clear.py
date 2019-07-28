#!/usr/bin/python2
# -*- coding: UTF-8 -*-
import os

""" 主入口函数 """
def main():
    os.popen("iptables -t nat -F")

if "__main__" == __name__:
    main()
