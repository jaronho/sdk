#include <iostream>

#include "test_bytearray.hpp"
#include "test_charset.hpp"
#include "test_datetime.hpp"
#include "test_filecopy.hpp"
#include "test_filesystem.hpp"
#include "test_module.hpp"
#include "test_net.hpp"
#include "test_process.hpp"
#include "test_system.hpp"
#include "test_timewatch.hpp"
#include "test_util.hpp"

int main(int argc, char** argv)
{
    testBytearry();
#if 0
        testFileCopy(argc, argv);
#else
#if 0
        testCharset(argc, argv);
#else
    testDateTime();
    testFilesystem();
    testModule();
    testNet();
    testProcess();
    testSystem();
    testTimewatch();
    testUtil();
#endif
#endif
    return 0;
}
