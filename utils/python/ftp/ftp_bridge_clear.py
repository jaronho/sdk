#!/usr/bin/python2
# -*- coding: UTF-8 -*-
import argparse
import os
import sys

""" 主入口函数 """
def main():
    if 1 == len(sys.argv):
        sys.argv.append("-h")
    parser = argparse.ArgumentParser(description="用于清空FTP桥接策略",
                                     usage=os.path.basename(__file__) + " [-h]"
    parser.parse_args()
    args = vars(parser.parse_args())
    os.popen("iptables -F")

if "__main__" == __name__:
    main()
