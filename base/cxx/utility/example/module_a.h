#pragma once

#include "../utility/module/module_manager.h"

class ModuleA : public utility::Module
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