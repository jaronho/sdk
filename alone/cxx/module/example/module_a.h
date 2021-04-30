#pragma once

#include "../module/module_manager.h"

class ModuleA : public module::Module
{
public:
    ModuleA();
    ~ModuleA();
    void onStart() override;
    void onResume() override;
    void onPause() override;
    void onStop() override;
    void printA();
};