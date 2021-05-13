#pragma once

#include "module/module_manager.h"

class ModuleC : public module::Module
{
public:
    virtual void print() = 0;
};