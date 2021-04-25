#include "module_b.h"
#include <iostream>

REGISTER_MODULE(ModuleB);
ModuleB::ModuleB()
{
    std::cout << "===== B created" << std::endl;
}

ModuleB::~ModuleB()
{
    std::cout << "===== B destroyed" << std::endl;
}

void ModuleB::onStart()
{
    std::cout << "===== B start" << std::endl;
}

void ModuleB::onResume()
{
    std::cout << "===== B resume" << std::endl;
}

void ModuleB::onPause()
{
    std::cout << "===== B pause" << std::endl;
}

void ModuleB::onStop()
{
    std::cout << "===== B stop" << std::endl;
}

void ModuleB::printB()
{
    std::cout << "===== B print" << std::endl;
}
