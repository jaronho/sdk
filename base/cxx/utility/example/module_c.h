#pragma once

#include "../utility/module/module_manager.h"

class ModuleC : public utility::Module
{
public:
    virtual void print() = 0;
};