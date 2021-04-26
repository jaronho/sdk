#include <iostream>
#include "module_a.h"
#include "module_b.h"
#include "module_c.h"

int main()
{
    ModuleManager::getInstance().setLogFunc([](const std::string& msg) { std::cout << msg << std::endl; });
    std::cout << std::endl << std::endl;
    ModuleManager::getInstance().create();
    std::cout << std::endl << std::endl;
    ModuleManager::getInstance().start();
    std::cout << std::endl << std::endl;

    GET_MODULE(ModuleA)->printA();
    GET_MODULE(ModuleB)->printB();
    GET_MODULE(ModuleC)->print();

    std::cout << std::endl << std::endl;
    ModuleManager::getInstance().stop();
    std::cout << std::endl << std::endl;
    ModuleManager::getInstance().destroy();
}
