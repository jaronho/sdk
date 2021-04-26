#pragma once

#include "module_c.h"

class ModuleCImpl : public ModuleC
{
public:
    ModuleCImpl();
    ~ModuleCImpl();
    void onStart() override;
    void onResume() override;
    void onPause() override;
    void onStop() override;
    void print() override;
};