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

void test1()
{
    std::cout << "================================= test1" << std::endl;
    std::string errDesc;
    nlohmann::json obj;
    if (!nlohmann::stringToJson(jsonStr, obj, &errDesc))
    {
        std::cout << "parse json string fail: " << errDesc << std::endl;
        return;
    }
    std::cout << obj.dump(4) << std::endl;
    std::cout << "---------------------------------" << std::endl;
    /* 获取子项 */
    std::string company;
    if (nlohmann::getJsonValue<std::string>(obj, "company", company, &errDesc))
    {
        std::cout << "company = " << company << std::endl;
    }
    else
    {
        std::cout << "get key [company] failed: " << errDesc << std::endl;
    }
    int members;
    if (nlohmann::getJsonValue<int>(obj, "members", members, &errDesc))
    {
        std::cout << "members = " << members << std::endl;
    }
    else
    {
        std::cout << "get key [members] failed: " << errDesc << std::endl;
    }
    bool ipo;
    if (nlohmann::getJsonValue<bool>(obj, "ipo", ipo, &errDesc))
    {
        std::cout << "ipo = " << ipo << std::endl;
    }
    else
    {
        std::cout << "get key [ipo] failed: " << errDesc << std::endl;
    }
    nlohmann::json departments;
    if (nlohmann::getJsonObject(obj, "departments", departments, &errDesc))
    {
        if (departments.is_array())
        {
            std::cout << "departments = " << std::endl;
            for (size_t i = 0; i < departments.size(); ++i)
            {
                std::cout << "    ----- [" << (i + 1) << "]" << std::endl;
                std::string name;
                if (nlohmann::getJsonValue<std::string>(departments[i], "name", name, &errDesc))
                {
                    std::cout << "    name = " << name << std::endl;
                }
                else
                {
                    std::cout << "get key [name] failed: " << errDesc << std::endl;
                }
                int members;
                if (nlohmann::getJsonValue<int>(departments[i], "members", members, &errDesc))
                {
                    std::cout << "    members = " << members << std::endl;
                }
                else
                {
                    std::cout << "get key [members] failed: " << errDesc << std::endl;
                }
                std::string param;
                if (nlohmann::getJsonValue<std::string>(departments[i], "param", param, &errDesc))
                {
                    std::cout << "    param = " << param << std::endl;
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

void test2()
{
    std::cout << "================================= test1" << std::endl;
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
    test1();
    test2();
}
