#include "inifile/ini_reader.h"
#include "inifile/ini_writer.h"

#include <iomanip>
#include <iostream>

bool copyFile(const std::string& srcFilePath, const std::string& destFilePath)
{
    if (srcFilePath.empty() || destFilePath.empty() || srcFilePath == destFilePath)
    {
        return false;
    }
#ifdef _WIN32
    FILE* fileSrc;
    if (0 != fopen_s(&fileSrc, srcFilePath.c_str(), "r"))
    {
#else
    FILE* fileSrc = fopen(srcFilePath.c_str(), "r");
    if (!fileSrc)
    {
#endif
        return false;
    }
#ifdef _WIN32
    FILE* fileDest;
    if (0 != fopen_s(&fileDest, destFilePath.c_str(), "w+"))
    {
#else
    FILE* fileDest = fopen(destFilePath.c_str(), "w+");
    if (!fileDest)
    {
#endif
        fclose(fileSrc);
        return false;
    }
    char buffer[1024] = {0};
    unsigned long len = 0;
    while ((len = fread(buffer, 1, 1024, fileSrc)) > 0)
    {
        fwrite(buffer, 1, len, fileDest);
    }
    fclose(fileSrc);
    fclose(fileDest);
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
    if (ir.open("../persion.ini"))
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
    if (iw.open("num.ini"))
    {
        std::string section;
        std::string key;

        section = std::string();
        iw.setInt(section, "count", iw.getInt(section, "count") + 1);

        if (!iw.setString(section, "describe", "hello"))
        {
            std::cout << "can't find key [describe]" << std::endl;
        }

        iw.save();
    }
    else
    {
        std::cout << "can't open file test.ini" << std::endl;
    }
}

/**
 * @brief 测试写本地INI文件(字段不存在时自动创建)
 */
void testWrite2()
{
    std::cout << "===================================== test write" << std::endl;
    ini::IniWriter iw;
    if (iw.open("test.ini"))
    {
        iw.setAllowAutoCreate();

        std::string section;
        std::string key;

        section = std::string();
        iw.setSectionComment(section, "测试文件信息");
        iw.setString(section, "title", "test information");

        section = "MAJOR_INFO";
        iw.setSectionComment(section, "#基础信息");

        key = "name";
        iw.setComment(section, key, "#姓名");
        iw.setString(section, key, "Marry Kaly");

        key = "male";
        iw.setComment(section, key, "#性别");
        iw.setBool(section, key, false);

        key = "age";
        iw.setComment(section, key, "#年龄");
        iw.setInt(section, key, 26);

        key = "identify";
        iw.setULongLong(section, key, 12345678901234567890);

        key = "weight";
        iw.setComment(section, key, "#体重");
        iw.setFloat(section, key, 43.6);

        section = "MINOR_INFO";
        iw.setSectionComment(section, "#次要信息");

        key = "company";
        iw.setComment(section, key, "#公司");
        iw.setString(section, key, "Bona Brother");

        key = "salary";
        iw.setComment(section, key, "#工资");
        iw.setDouble(section, key, 654321.987654321);

        iw.save();
    }
    else
    {
        std::cout << "can't open file test.ini" << std::endl;
    }
}

int main()
{
    testRead();
    std::cout << std::endl;
    testWrite1();
    std::cout << std::endl;
    testWrite2();
    std::cout << std::endl;
    return 0;
}