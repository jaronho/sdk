#pragma once

#include "../utility/module/module_manager.h"

class ModuleB : public utility::Module
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