#pragma once

#include "../module/module_manager.h"

class ModuleB : public module::Module
{
public:
    ModuleB();
    ~ModuleB();
    void onStart() override;
    void onResume() override;
    void onPause() override;
    void onStop() override;
    void printB();
};