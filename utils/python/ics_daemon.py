#!/usr/bin/python2
# -*- coding: UTF-8 -*- 
import ConfigParser
import getpass
import logging
import os
import psutil
import sys
import syslog
import thread
import time

def printMsg(str):
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()) + " " + str)
    syslog.syslog(str)

def screenRecord(minute, path):
    printMsg("录屏时间: " + minute + ", 录屏路径: " + path)
    os.system("python "+sys.path[0]+"/screenrecord.py -m " + minute + " -p " + path)

def keyboardRecord(minute, path, port):
    printMsg("键盘记录时间: " + minute + ", 键盘记录路径: " + path + ", 键盘串口: " + port)
    os.system("python "+sys.path[0]+"/com.py -m " + minute + " -p " + path + " -port " + port)

def listenClient(name, fullPath):
    while True:
        clientExist = False
        for pid in psutil.pids():
            p = psutil.Process(pid)
            if name == p.name():
                clientExist = True
                break
        if False == clientExist:
            if True == os.path.exists(fullPath):
                os.system("chmod 777 " + fullPath)
                if True == os.access(fullPath, os.X_OK):
                    printMsg("工控客户端未启动, 准备启动")
                    os.system(fullPath)
                else:
                    printMsg("工控客户端程序文件 \"" + fullPath + "\" 不具有可执行权限, 无法启动")
            else:
                printMsg("工控客户端程序文件 \"" + fullPath + "\" 不存在, 无法启动")
        time.sleep(1)

def main(version):
    syslog.openlog("ics_daemon", syslog.LOG_PID)
    count = 0
    for pid in psutil.pids():
        p = psutil.Process(pid)
        if (str(p.cmdline()).find("ics_daemon") != -1):
            count += 1
            if count >= 2:  # ics_daemon被启动了两次
                printMsg("工控客户端守护进程已经启动, 无需重新启动, 当前用户: " + getpass.getuser())
                syslog.closelog()
                return
    printMsg("工控客户端守护进程启动, 当前用户: " + getpass.getuser())
    currentPath = os.path.dirname(os.path.realpath(__file__))
    configPath = os.path.join(currentPath, "config.ini")
    conf = ConfigParser.ConfigParser()
    conf.read(configPath)
    # 读取录屏配置参数
    printMsg("读取录屏配置参数")
    screenRecordEnable = conf.get("CONFIG", "SCREEN_RECORD_ENALBE")
    screenRecordMinute = conf.get("CONFIG", "SCREEN_RECORD_MINUTE")
    screenRecordPath = conf.get("CONFIG", "SCREEN_RECORD_PATH")
    # 读取键盘记录配置参数
    printMsg("读取键盘记录配置参数")
    keyboardRecordEnable = conf.get("CONFIG", "KEYBOARD_RECORD_ENABLE")
    keyboardRecordMinute = conf.get("CONFIG", "KEYBOARD_RECORD_MINUTE")
    keyboardRecordPath = conf.get("CONFIG", "KEYBOARD_RECORD_PATH")
    serialKeyboardPort = conf.get("CONFIG", "SERIAL_KEYBOARD")
    # 判断是否启用录屏
    if "1" == screenRecordEnable:
        printMsg("启用录屏")
        thread.start_new_thread(screenRecord, (screenRecordMinute, screenRecordPath))
    else:
        printMsg("禁用录屏")
    # 判断是否启用键盘记录
    if "1" == keyboardRecordEnable:
        printMsg("启用键盘记录")
        thread.start_new_thread(keyboardRecord, (keyboardRecordMinute, keyboardRecordPath, serialKeyboardPort))
    else:
        printMsg("禁用键盘记录")
    # 监听工控客户端进程
    printMsg("开始监听工控客户端进程")
    clientName = "ics_client" + version
    clientFullpath = currentPath + "/" + clientName
    thread.start_new_thread(listenClient, (clientName, clientFullpath))
    logging.info(listenClient)
    while True:
        time.sleep(1)
    syslog.closelog()

if "__main__" == __name__:
    main("")
