#pragma once

#include "../utilitiy/module/module_manager.h"

class ModuleB : public utilitiy::Module
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