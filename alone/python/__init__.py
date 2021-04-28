# -*- coding: utf-8 -*-
import sys

# 判断是否python2
def isPython2():
    return hasattr(str, "decode")

# python2版本默认编码为:ascii,设置默认为:utf-8
if isPython2() and "utf-8" != sys.getdefaultencoding():
    reload(sys)
    sys.setdefaultencoding("utf-8")
