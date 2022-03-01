#pragma once
#include <fstream>
#include <iomanip>
#include <iostream>

#include "../fileparse/ini/ini_reader.h"
#include "../fileparse/ini/ini_writer.h"
#include "config.h"

bool copyFile(const std::string& srcFilePath, const std::string& destFilePath)
{
    if (srcFilePath.empty() || destFilePath.empty() || srcFilePath == destFilePath)
    {
        return false;
    }
    std::fstream srcFile(srcFilePath, std::ios::in | std::ios::binary);
    if (!srcFile.is_open())
    {
        return false;
    }
    std::fstream destFile(destFilePath, std::ios::out | std::ios::binary);
    if (!destFile.is_open())
    {
        srcFile.close();
        return false;
    }
    char buffer[1024] = {0};
    unsigned long len = 0;
    while (!srcFile.eof())
    {
        srcFile.read(buffer, 1024);
        destFile.write(buffer, srcFile.gcount());
    }
    srcFile.close();
    destFile.close();
    return true;
}

void printBool(ini::IniReader ir, const std::string& section, const std::string& key)
{
    if (ir.hasKey(section, key))
    {
        std::string comment;
        ir.getComment(section, key, comment);
        std::cout << "    " << key << std::endl;
        std::cout << "    {" << std::endl;
        std::cout << "        comment:" << comment << std::endl;
        std::cout << "        value:" << ir.getBool(section, key) << std::endl;
        std::cout << "    }" << std::endl;
    }
    else
    {
        std::cout << "    not exist key [" << key << "]" << std::endl;
    }
}

void printInt(ini::IniReader ir, const std::string& section, const std::string& key)
{
    if (ir.hasKey(section, key))
    {
        std::string comment;
        ir.getComment(section, key, comment);
        std::cout << "    " << key << std::endl;
        std::cout << "    {" << std::endl;
        std::cout << "        comment:" << comment << std::endl;
        std::cout << "        value:" << ir.getInt(section, key) << std::endl;
        std::cout << "    }" << std::endl;
    }
    else
    {
        std::cout << "    not exist key [" << key << "]" << std::endl;
    }
}

void printFloat(ini::IniReader ir, const std::string& section, const std::string& key)
{
    if (ir.hasKey(section, key))
    {
        std::string comment;
        ir.getComment(section, key, comment);
        std::cout << "    " << key << std::endl;
        std::cout << "    {" << std::endl;
        std::cout << "        comment:" << comment << std::endl;
        std::cout << "        value:" << std::fixed << ir.getFloat(section, key) << std::endl;
        std::cout << "    }" << std::endl;
    }
    else
    {
        std::cout << "    not exist key [" << key << "]" << std::endl;
    }
}

void printDouble(ini::IniReader ir, const std::string& section, const std::string& key)
{
    if (ir.hasKey(section, key))
    {
        std::string comment;
        ir.getComment(section, key, comment);
        std::cout << "    " << key << std::endl;
        std::cout << "    {" << std::endl;
        std::cout << "        comment:" << comment << std::endl;
        std::cout << "        value:" << std::setprecision(10) << ir.getDouble(section, key) << std::endl;
        std::cout << "    }" << std::endl;
    }
    else
    {
        std::cout << "    not exist key [" << key << "]" << std::endl;
    }
}

void printULongLong(ini::IniReader ir, const std::string& section, const std::string& key)
{
    if (ir.hasKey(section, key))
    {
        std::string comment;
        ir.getComment(section, key, comment);
        std::cout << "    " << key << std::endl;
        std::cout << "    {" << std::endl;
        std::cout << "        comment:" << comment << std::endl;
        std::cout << "        value:" << ir.getULongLong(section, key) << std::endl;
        std::cout << "    }" << std::endl;
    }
    else
    {
        std::cout << "    not exist key [" << key << "]" << std::endl;
    }
}

void printString(ini::IniReader ir, const std::string& section, const std::string& key)
{
    if (ir.hasKey(section, key))
    {
        std::string comment;
        ir.getComment(section, key, comment);
        std::cout << "    " << key << std::endl;
        std::cout << "    {" << std::endl;
        std::cout << "        comment:" << comment << std::endl;
        std::cout << "        value:" << ir.getString(section, key) << std::endl;
        std::cout << "    }" << std::endl;
    }
    else
    {
        std::cout << "    not exist key [" << key << "]" << std::endl;
    }
}

/**
 * @brief 测试读本地INI文件
 */
void testRead()
{
    std::cout << "===================================== test read" << std::endl;
    ini::IniReader ir;
    std::string errorDesc;
    if (0 == ir.open("../persion.ini", errorDesc))
    {
        std::string section;
        std::string key;

        section = std::string();
        if (ir.hasSection(section))
        {
            std::cout << "===== section [" << section << "]" << std::endl;
            std::cout << "{" << std::endl;
            printString(ir, section, "title");
            std::cout << "}" << std::endl;
        }
        else
        {
            std::cout << "can't find section [" << section << "]" << std::endl;
        }

        section = "BASE_INFO";
        if (ir.hasSection(section))
        {
            std::cout << "===== section [" << section << "]" << std::endl;
            std::cout << "{" << std::endl;
            printString(ir, section, "name");
            printBool(ir, section, "male");
            printInt(ir, section, "age");
            printULongLong(ir, section, "idendify");
            printFloat(ir, section, "weight");
            std::cout << "}" << std::endl;
        }
        else
        {
            std::cout << "can't find section [" << section << "]" << std::endl;
        }

        section = "EXTRA_INFO";
        if (ir.hasSection(section))
        {
            std::cout << "===== section [" << section << "]" << std::endl;
            std::cout << "{" << std::endl;
            printString(ir, section, "company");
            printDouble(ir, section, "salary");
            printString(ir, section, "interest");
            printString(ir, section, "speciality");
            std::cout << "}" << std::endl;
        }
        else
        {
            std::cout << "can't find section [" << section << "]" << std::endl;
        }

        section = "OTHER_INFO";
        if (ir.hasSection(section))
        {
            std::cout << "===== section [" << section << "]" << std::endl;
            std::cout << "{" << std::endl;
            std::cout << "}" << std::endl;
        }
        else
        {
            std::cout << "can't find section [" << section << "]" << std::endl;
        }
    }
    else
    {
        std::cout << "can't open file persion.ini" << std::endl;
    }
}

/**
 * @brief 测试写本地INI文件(需要字段都已经存在)
 */
void testWrite1()
{
    copyFile("../num.ini", "num.ini"); /* 拷贝测试文件 */
    std::cout << "===================================== test write" << std::endl;
    ini::IniWriter iw;
    std::string errorDesc;
    if (0 == iw.open("num.ini", errorDesc))
    {
        std::string section;
        std::string key;

        section = std::string();
        iw.setValue(section, "count", iw.getInt(section, "count") + 1);

        if (!iw.setValue(section, "describe", "hello"))
        {
            std::cout << "can't find key [describe]" << std::endl;
        }

        if (iw.save())
        {
            std::cout << "save file num.ini ok" << std::endl;
        }
        else
        {
            std::cout << "save file num.ini fail" << std::endl;
        }
    }
    else
    {
        std::cout << "can't open file num.ini" << std::endl;
    }
}

/**
 * @brief 测试写本地INI文件(字段不存在时自动创建)
 */
void testWrite2()
{
    std::cout << "===================================== test write" << std::endl;
    ini::IniWriter iw;
    std::string errorDesc;
    if (0 == iw.open("test.ini", errorDesc))
    {
        iw.setAllowAutoCreate();

        std::string section;
        std::string key;

        section = std::string();
        iw.setSectionComment(section, "测试文件信息");
        iw.setValue(section, "title", "test information");

        section = "MAJOR_INFO";
        iw.setSectionComment(section, "#基础信息");

        key = "name";
        iw.setComment(section, key, "#姓名");
        iw.setValue(section, key, "Marry Kaly");

        key = "male";
        iw.setComment(section, key, "#性别");
        iw.setValue(section, key, false);

        key = "age";
        iw.setComment(section, key, "#年龄");
        iw.setValue(section, key, 26);

        key = "identify";
        iw.setValue(section, key, (unsigned long long)12345678901234567890);

        key = "weight";
        iw.setComment(section, key, "#体重");
        iw.setValue(section, key, 43.6);

        section = "MINOR_INFO";
        iw.setSectionComment(section, "#次要信息");

        key = "company";
        iw.setComment(section, key, "#公司");
        iw.setValue(section, key, "Bona Brother");

        key = "salary";
        iw.setComment(section, key, "#工资");
        iw.setValue(section, key, 654321.987654321);

        if (iw.save())
        {
            std::cout << "save file test.ini ok" << std::endl;
        }
        else
        {
            std::cout << "save file test.ini fail" << std::endl;
        }
    }
    else
    {
        std::cout << "can't open file test.ini" << std::endl;
    }
}

/**
 * @brief 测试恢复默认设置
 */
void testRestore()
{
    std::cout << "===================================== test restore" << std::endl;
    Config cfg;
    if (cfg.init("..", true))
    {
        cfg.setValue(cfgkey::SERVER_IP, "192.168.5.3");
        cfg.save();
        std::cout << "modify server ip to " << cfg.getValue(cfgkey::SERVER_IP).toString() << std::endl;
        cfg.restoreFactory();
        std::cout << "restore server ip to " << cfg.getValue(cfgkey::SERVER_IP).toString() << std::endl;
    }
}

void testIni()
{
    printf("\n============================== test ini =============================\n");
    testRead();
    std::cout << std::endl;
    testWrite1();
    std::cout << std::endl;
    testWrite2();
    std::cout << std::endl;
    testRestore();
    std::cout << std::endl;
}