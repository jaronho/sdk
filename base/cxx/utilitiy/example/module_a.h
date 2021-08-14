#pragma once

#include "../utilitiy/module/module_manager.h"

class ModuleA : public utilitiy::Module
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