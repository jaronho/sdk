#include <iostream>

#include "test_filesystem.hpp"
#include "test_process.hpp"
#include "test_system.hpp"

int main(int argc, char** argv)
{
    testFilesystem();
    testProcess();
    testSystem();
    return 0;
}
