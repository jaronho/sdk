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

void testJsonRead1()
{
    std::cout << "================================= testJsonRead1" << std::endl;
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
    if (nlohmann::getter<std::string>(obj, "company", company, &errDesc))
    {
        std::cout << "company = " << company << std::endl;
    }
    else
    {
        std::cout << "get key [company] failed: " << errDesc << std::endl;
    }
    int members;
    if (nlohmann::getter<int>(obj, "members", members, &errDesc))
    {
        std::cout << "members = " << members << std::endl;
    }
    else
    {
        std::cout << "get key [members] failed: " << errDesc << std::endl;
    }
    bool ipo;
    if (nlohmann::getter<bool>(obj, "ipo", ipo, &errDesc))
    {
        std::cout << "ipo = " << ipo << std::endl;
    }
    else
    {
        std::cout << "get key [ipo] failed: " << errDesc << std::endl;
    }
    nlohmann::json departments;
    if (nlohmann::getter(obj, "departments", departments, &errDesc))
    {
        if (departments.is_array())
        {
            std::cout << "departments = " << std::endl;
            for (size_t i = 0; i < departments.size(); ++i)
            {
                std::cout << "    ----- [" << (i + 1) << "]" << std::endl;
                std::string name;
                if (nlohmann::getter<std::string>(departments[i], "name", name, &errDesc))
                {
                    std::cout << "        name = " << name << std::endl;
                }
                else
                {
                    std::cout << "get key [name] failed: " << errDesc << std::endl;
                }
                int members;
                if (nlohmann::getter<int>(departments[i], "members", members, &errDesc))
                {
                    std::cout << "        members = " << members << std::endl;
                }
                else
                {
                    std::cout << "get key [members] failed: " << errDesc << std::endl;
                }
                std::string param;
                if (nlohmann::getter<std::string>(departments[i], "param", param, &errDesc))
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

void testJsonRead2()
{
    std::cout << "================================= testJsonRead2" << std::endl;
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

void testJsonRead3()
{
    std::cout << "================================= testJsonRead3" << std::endl;
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
    auto company = nlohmann::getter<std::string>(obj, "company");
    std::cout << "company = " << company << std::endl;
    auto members = nlohmann::getter<int>(obj, "members");
    std::cout << "members = " << members << std::endl;
    auto ipo = nlohmann::getter<bool>(obj, "ipo");
    std::cout << "ipo = " << ipo << std::endl;
    auto departments = nlohmann::getter(obj, "departments");
    std::cout << "departments = " << std::endl;
    int index = 0;
    for (auto item : departments)
    {
        std::cout << "    ----- [" << (++index) << "]" << std::endl;
        auto name = nlohmann::getter<std::string>(item, "name");
        std::cout << "        name = " << name << std::endl;
        auto members = nlohmann::getter<int>(item, "members");
        std::cout << "        members = " << members << std::endl;
        auto param = nlohmann::getter<std::string>(item, "param");
        std::cout << "        param = " << param << std::endl;
    }
}

void testJsonWrite1()
{
    std::cout << "================================= testJsonWrite1" << std::endl;
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
    obj["empty_object"] = nlohmann::json::object();
    obj["empty_array"] = nlohmann::json::array();
    std::cout << obj.dump(4) << std::endl;
}

void testJsonWrite2()
{
    std::cout << "================================= testJsonWrite2" << std::endl;
    nlohmann::json obj;
    nlohmann::setter<std::string>(obj, "country", "China");
    nlohmann::setter<int>(obj, "people", 1400000000);
    nlohmann::setter<bool>(obj, "developing", true);
    nlohmann::json provinces;
    nlohmann::json fj;
    nlohmann::setter(fj, "name", "FuJian");
    nlohmann::setter(fj, "capital", "FuZhou");
    nlohmann::setter(fj, "celebrity", "LinZeXu");
    nlohmann::setter(fj, "yanhai", true);
    provinces.emplace_back(fj);
    nlohmann::json gd;
    gd["name"] = "GuangDong";
    gd["capital"] = "GuangZhou";
    gd["gdp"] = 1;
    nlohmann::append(provinces, gd);
    obj["provinces"] = provinces;
    obj["empty_object"] = nlohmann::json::object();
    nlohmann::setter(obj, "empty_array", nlohmann::json::array());
    std::cout << obj.dump(4) << std::endl;
}

void testJson()
{
    printf("\n============================== test json =============================\n");
    testJsonRead1();
    testJsonRead2();
    testJsonRead3();
    testJsonWrite1();
    testJsonWrite2();
}
