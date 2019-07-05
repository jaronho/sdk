#!/usr/bin/python2
# -*- coding: UTF-8 -*-
import argparse
import binascii
import math
import os
import sys

"""
 * Brief:   文件类型列表
 * Param:   无
 * Return:  类型列表
"""
def typeList():  
    return {
        "3026B2758E66CF11"                  : "asf",
        "41564920"                          : "avi",
        "424D"                              : "bmp",
        "CFAD12FEC5FD746F"                  : "dbx",
        "D0CF11E0"                          : "doc,xls",
        "504B0304"                          : "docx",
        "41433130"                          : "dwg",
        "44656C69766572792D646174653A"      : "eml",
        "47494638"                          : "gif",
        "1F8B08"                            : "gz",
        "68746D6C3E"                        : "html",
        "FFD8FF"                            : "jpeg,jpg",
        "5374616E64617264204A"              : "mdb",
        "4D546864"                          : "mid",
        "6D6F6F76"                          : "mov",
        "000001B3"                          : "mpg",
        "000001BA"                          : "mpg",
        "255044462D312E"                    : "pdf",
        "4D5A"                              : "pe",
        "89504E47"                          : "png",
        "252150532D41646F6265"              : "ps",
        "38425053"                          : "psd",
        "2142444E"                          : "pst",
        "E3828596"                          : "pwl",
        "AC9EBD8F"                          : "qdf",
        "2E7261FD"                          : "ram",
        "52617221"                          : "rar",
        "2E524D46"                          : "rm",
        "7B5C727466"                        : "rtf",
        "49492A00"                          : "tif",
        "57415645"                          : "wav",
        "FF575043"                          : "wpd",
        "3C3F786D6C"                        : "xml",
        "504B0304"                          : "zip"
    }

"""
 * Brief:   判断文件是否以指定字节开头
 * Param:   file - 文件名或者文件句柄
 *          hexStr - 十六进制字符串.例如: FFD8FF 表示 JPEG
 * Return:  布尔值
"""
def isFileHeadWith(file, hexStr):
    try:
        byteNum = math.ceil(len(hexStr) / 2)
        if isinstance(file, str):
            with open(file, 'rb') as fp:
                headHex = fp.read(byteNum).hex().upper()
        else:
            file.seek(0)
            headHex = file.read(byteNum).hex().upper()
        return headHex.startswith(hexStr.upper())
    except:
        return False

"""
 * Brief:   获取文件类型
 * Param:   filename - 文件名
 * Return:  文件类型
"""
def filetype(filename):
    try:
        with open(filename, 'rb') as fp:
            tl = typeList()
            for hexCode in tl.keys():
                if isFileHeadWith(fp, hexCode):
                    return {"code": hexCode.upper(), "typestr": tl[hexCode]}
    except:
        return {"code": "", "typestr": "Unknown"}
    else:
        return {"code": "", "typestr": "Unknown"}

""" 主入口函数 """
def main():
    if 1 == len(sys.argv):
        sys.argv.append("-h")
    parser = argparse.ArgumentParser(description="用于查看文件类型",
                                     usage=os.path.basename(__file__) + " [-h] [-f FILE]")
    parser.add_argument('-f','--file',metavar="",type=str,help="指定文件. 例如: abc.txt")
    parser.parse_args()
    args = vars(parser.parse_args())
    # step1:必填参数判断
    if not args["file"]:
        print("--file 参数缺少")
        return
    # step2:打开文件
    try:
        print(filetype(args["file"]))
    except:
        print(sys.exc_info(), args["file"])

if "__main__" == __name__:
    main()
