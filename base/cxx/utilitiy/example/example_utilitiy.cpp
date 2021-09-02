#include <iostream>

#include "test_filecopy.hpp"
#include "test_filesystem.hpp"
#include "test_module.hpp"
#include "test_net.hpp"
#include "test_process.hpp"
#include "test_system.hpp"
#include "test_timewatch.hpp"

int main(int argc, char** argv)
{
#if 0
    testFileCopy(argc, argv);
#else
    testFilesystem();
    testModule();
    testNet();
    testProcess();
    testSystem();
    testTimewatch();
#endif
    return 0;
}
