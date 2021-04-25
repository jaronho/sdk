#include "module_a.h"
#include <iostream>

REGISTER_MODULE(ModuleA);
ModuleA::ModuleA()
{
    std::cout << "===== A created" << std::endl;
}

ModuleA::~ModuleA()
{
    std::cout << "===== A destroyed" << std::endl;
}

void ModuleA::onStart()
{
    std::cout << "===== A start" << std::endl;
}

void ModuleA::onResume()
{
    std::cout << "===== A resume" << std::endl;
}

void ModuleA::onPause()
{
    std::cout << "===== A pause" << std::endl;
}

void ModuleA::onStop()
{
    std::cout << "===== A stop" << std::endl;
}

void ModuleA::printA()
{
    std::cout << "===== A print" << std::endl;
}
