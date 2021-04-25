#include "module_c_impl.h"
#include <iostream>

REGISTER_MODULE_IMPL(ModuleC, ModuleCImpl);
ModuleCImpl::ModuleCImpl()
{
    std::cout << "===== C impl created" << std::endl;
}

ModuleCImpl::~ModuleCImpl()
{
    std::cout << "===== C impl destroyed" << std::endl;
}

void ModuleCImpl::onStart()
{
    std::cout << "===== C impl start" << std::endl;
}

void ModuleCImpl::onResume()
{
    std::cout << "===== C impl resume" << std::endl;
}

void ModuleCImpl::onPause()
{
    std::cout << "===== C impl pause" << std::endl;
}

void ModuleCImpl::onStop()
{
    std::cout << "===== C impl stop" << std::endl;
}

void ModuleCImpl::print()
{
    std::cout << "===== C impl print" << std::endl;
}
