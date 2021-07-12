#include <iostream>

#include "module_a.h"
#include "module_b.h"
#include "module_c.h"

int main()
{
    module::ModuleManager::getInstance().setLogFunc([](const std::string& msg) { std::cout << msg << std::endl; });
    std::cout << std::endl << std::endl;
    module::ModuleManager::getInstance().create();
    std::cout << std::endl << std::endl;
    module::ModuleManager::getInstance().start();
    std::cout << std::endl << std::endl;

    GET_MODULE(ModuleA)->printA();
    GET_MODULE(ModuleB)->printB();
    GET_MODULE(ModuleC)->print();

    std::cout << std::endl << std::endl;
    module::ModuleManager::getInstance().stop();
    std::cout << std::endl << std::endl;
    module::ModuleManager::getInstance().destroy();
}
