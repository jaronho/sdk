#!/usr/bin/python2
# -*- coding: UTF-8 -*-
import argparse
import json
import logging
import os
import sys

""" 生成返回数据 """
def generateRetData(status=1, describe=""):
    jsonData = {
        "status":status,        # 状态值码:0.杀毒软件已安装,1.杀毒软件未安装
        "describe":describe,    # 状态码描述
        "details":[],           # 详细列表
        "infected":0,           # 病毒感染文件个数
        "error":0               # 杀毒时出错个数
    }
    return jsonData

""" 使用clamav杀毒 """
def clamavScan(path):
    jsonData = generateRetData()
    # step1:判断杀毒软件是否安装
    if 0 == len(os.popen("which clamdscan").read()):
        jsonData["describe"] = "can not find clamdscan"
        return json.dumps(jsonData, ensure_ascii=False)
    # step2:开始杀毒
    jsonData["status"] = 0
    cmd = "clamdscan --fdpass -i '%s' " % (path)
    output = os.popen(cmd).read().strip("\r\n")
    # step3:解析结果
    divideLine = "----------- SCAN SUMMARY -----------"
    pos = output.find(divideLine)
    if pos >= 0:
        detailList = filter(lambda x: 0 != len(x), output[0:pos].split("\n"))
        statusList = filter(lambda x: 0 != len(x), output[pos+len(divideLine):].split("\n"))
        for detail in detailList:
            if len(detail) > 0:
                jsonData["details"].append(detail)
        for status in statusList: 
            if "Infected files:".upper() in status.upper():
                jsonData["infected"] += int(status.split(":")[1])
            elif "Total errors:".upper() in status.upper():
                jsonData["error"] += int(status.split(":")[1])
    return json.dumps(jsonData, ensure_ascii=False)

""" 主入口函数 """
def main():
    logging.basicConfig(filename='antivirus.log', filemode="w", level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
    if 1 == len(sys.argv):
	    sys.argv.append("-h")
    parser = argparse.ArgumentParser(description="AntiVirus Helper",
                                     usage=os.path.basename(__file__) + " [-h] [-e ENGINE] [-p PATH]")
    parser.add_argument("-e","--engine",type=str,default="clamav",help="use which antivirus")
    parser.add_argument("-p","--path",type=str,help="input the path need to antivirus. eg. /var/local/data ")
    parser.parse_args()
    args = vars(parser.parse_args())
    # step1:参数判断
    if not args["path"]:
        print(generateRetData(2, "--path missing"))
        logging.error("--path missing")
        return
    # step2:路径判断
    if not os.path.exists(args["path"]):
        print(generateRetData(3, "'" + args["path"] + "' is not exist"))
        logging.error("'" + args["path"] + "' is not exist")
        return
    # step3:引擎选择
    logging.info("start scan " + args["path"])
    if args["engine"].upper() == "clamav".upper():
        jsonContent = clamavScan(args["path"])
        print(jsonContent)
        logging.info(jsonContent)
    else:
        print(generateRetData(4, "not support engine '" + args["engine"] + "'"))
        logging.error("not support engine '" + args["engine"] + "'")

if "__main__" == __name__:
    main()

