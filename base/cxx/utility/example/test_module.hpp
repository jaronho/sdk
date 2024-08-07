#pragma once
#include <iostream>

#include "module_a.h"
#include "module_b.h"
#include "module_c.h"

void testModule()
{
    printf("\n============================== test module =============================\n");
    utility::ModuleManager::getInstance().setLogFunc([](const std::string& msg) { std::cout << msg << std::endl; });
    std::cout << std::endl << std::endl;
    utility::ModuleManager::getInstance().create();
    std::cout << std::endl << std::endl;
    utility::ModuleManager::getInstance().start();
    std::cout << std::endl << std::endl;

    GET_MODULE(ModuleA)->printA();
    GET_MODULE(ModuleB)->printB();
    GET_MODULE(ModuleC)->print();

    std::cout << std::endl << std::endl;
    utility::ModuleManager::getInstance().stop();
    std::cout << std::endl << std::endl;
    utility::ModuleManager::getInstance().destroy();
}
