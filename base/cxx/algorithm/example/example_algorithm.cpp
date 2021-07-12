#include <iostream>

#include "test_base64.hpp"
#include "test_md5.hpp"
#include "test_rc4.hpp"
#include "test_sm3.hpp"
#include "test_snowflake.hpp"

int main(int argc, char** argv)
{
    testBase64();
    testMd5();
    testRc4();
    testSm3();
    testSnowflake();
    return 0;
}
