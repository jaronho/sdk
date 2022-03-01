#pragma once
#include <iostream>

#include "../fileparse/nlohmann/helper.hpp"

const std::string jsonStr = "{"
                            "   \"company\": \"MS\","
                            "   \"members\": 1024,"
                            "   \"ipo\": true,"
                            "   \"departments\":"
                            "   ["
                            "       {"
                            "           \"name\": \"xingzheng\","
                            "           \"members\": 27,"
                            "           \"param\": \"aaa\""
                            "       },"
                            "       {"
                            "           \"name\": \"yanfa\","
                            "           \"members\": 679,"
                            "           \"param\": 123"
                            "       },"
                            "       {"
                            "           \"name\": \"xiaoshou\","
                            "           \"members\": 107"
                            "       }"
                            "   ]"
                            "}";

void testRead1()
{
    std::cout << "================================= testRead1" << std::endl;
    std::string errDesc;
    nlohmann::json obj;
    if (!nlohmann::stringToObject(jsonStr, obj, &errDesc))
    {
        std::cout << "parse json string fail: " << errDesc << std::endl;
        return;
    }
    std::cout << obj.dump(4) << std::endl;
    std::cout << "---------------------------------" << std::endl;
    /* 获取子项 */
    std::string company;
    if (nlohmann::getChild<std::string>(obj, "company", company, &errDesc))
    {
        std::cout << "company = " << company << std::endl;
    }
    else
    {
        std::cout << "get key [company] failed: " << errDesc << std::endl;
    }
    int members;
    if (nlohmann::getChild<int>(obj, "members", members, &errDesc))
    {
        std::cout << "members = " << members << std::endl;
    }
    else
    {
        std::cout << "get key [members] failed: " << errDesc << std::endl;
    }
    bool ipo;
    if (nlohmann::getChild<bool>(obj, "ipo", ipo, &errDesc))
    {
        std::cout << "ipo = " << ipo << std::endl;
    }
    else
    {
        std::cout << "get key [ipo] failed: " << errDesc << std::endl;
    }
    nlohmann::json departments;
    if (nlohmann::getChild(obj, "departments", departments, &errDesc))
    {
        if (departments.is_array())
        {
            std::cout << "departments = " << std::endl;
            for (size_t i = 0; i < departments.size(); ++i)
            {
                std::cout << "    ----- [" << (i + 1) << "]" << std::endl;
                std::string name;
                if (nlohmann::getChild<std::string>(departments[i], "name", name, &errDesc))
                {
                    std::cout << "        name = " << name << std::endl;
                }
                else
                {
                    std::cout << "get key [name] failed: " << errDesc << std::endl;
                }
                int members;
                if (nlohmann::getChild<int>(departments[i], "members", members, &errDesc))
                {
                    std::cout << "        members = " << members << std::endl;
                }
                else
                {
                    std::cout << "get key [members] failed: " << errDesc << std::endl;
                }
                std::string param;
                if (nlohmann::getChild<std::string>(departments[i], "param", param, &errDesc))
                {
                    std::cout << "        param = " << param << std::endl;
                }
                else
                {
                    std::cout << "get key [param] failed: " << errDesc << std::endl;
                }
            }
        }
        else
        {
            std::cout << "departments is not array" << std::endl;
        }
    }
    else
    {
        std::cout << "get key [departments] failed: " << errDesc << std::endl;
    }
}

void testRead2()
{
    std::cout << "================================= testRead2" << std::endl;
    std::string errDesc;
    nlohmann::json obj;
    if (!nlohmann::stringToObject(jsonStr, obj, &errDesc))
    {
        std::cout << "parse json string fail: " << errDesc << std::endl;
        return;
    }
    std::cout << obj.dump(4) << std::endl;
    std::cout << "---------------------------------" << std::endl;
    /* 获取子项 */
    std::string company = obj["company"];
    std::cout << "company = " << company << std::endl;
    int members = obj["members"];
    std::cout << "members = " << members << std::endl;
    bool ipo = obj["ipo"];
    std::cout << "ipo = " << ipo << std::endl;
    std::cout << "departments = " << std::endl;
    int index = 0;
    for (auto item : obj["departments"])
    {
        std::cout << "    ----- [" << (index++) << "]" << std::endl;
        std::string name = item["name"];
        std::cout << "        name = " << name << std::endl;
        int members = item["members"];
        std::cout << "        members = " << members << std::endl;
        try
        {
            std::string param = item["param"];
            std::cout << "        param = " << param << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cout << "param parse fail, " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cout << "param parse fail, unknow error" << std::endl;
        }
    }
}

void testRead3()
{
    std::cout << "================================= testRead3" << std::endl;
    std::string errDesc;
    nlohmann::json obj;
    if (!nlohmann::stringToObject(jsonStr, obj, &errDesc))
    {
        std::cout << "parse json string fail: " << errDesc << std::endl;
        return;
    }
    std::cout << obj.dump(4) << std::endl;
    std::cout << "--------------------------------- " << obj.size() << std::endl;
    /* 获取子项 */
    auto company = nlohmann::getChild<std::string>(obj, "company");
    std::cout << "company = " << company << std::endl;
    auto members = nlohmann::getChild<int>(obj, "members");
    std::cout << "members = " << members << std::endl;
    auto ipo = nlohmann::getChild<bool>(obj, "ipo");
    std::cout << "ipo = " << ipo << std::endl;
    auto departments = nlohmann::getChild(obj, "departments");
    std::cout << "departments = " << std::endl;
    int index = 0;
    for (auto item : departments)
    {
        std::cout << "    ----- [" << (++index) << "]" << std::endl;
        auto name = nlohmann::getChild<std::string>(item, "name");
        std::cout << "        name = " << name << std::endl;
        auto members = nlohmann::getChild<int>(item, "members");
        std::cout << "        members = " << members << std::endl;
        auto param = nlohmann::getChild<std::string>(item, "param");
        std::cout << "        param = " << param << std::endl;
    }
}

void testWrite()
{
    std::cout << "================================= testWrite" << std::endl;
    nlohmann::json obj;
    obj["country"] = "China";
    obj["people"] = 1400000000;
    obj["developing"] = true;
    nlohmann::json provinces;
    nlohmann::json fj;
    fj["name"] = "FuJian";
    fj["capital"] = "FuZhou";
    fj["celebrity"] = "LinZeXu";
    provinces.emplace_back(fj);
    nlohmann::json gd;
    gd["name"] = "GuangDong";
    gd["capital"] = "GuangZhou";
    provinces.emplace_back(gd);
    obj["provinces"] = provinces;
    obj["empty_object"] = nlohmann::json().object();
    obj["empty_array"] = nlohmann::json().array();
    std::cout << obj.dump(4) << std::endl;
}

void testJson()
{
    printf("\n============================== test json =============================\n");
    testRead1();
    testRead2();
    testRead3();
    testWrite();
}
