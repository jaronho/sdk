#pragma once

#include "../utilitiy/module/module_manager.h"

class ModuleC : public utilitiy::Module
{
public:
    virtual void print() = 0;
};