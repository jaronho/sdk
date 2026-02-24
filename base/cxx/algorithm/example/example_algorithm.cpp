#include <iostream>

#include "test_base64.hpp"
#include "test_md5.hpp"
#include "test_rc4.hpp"
#include "test_sha1.hpp"
#include "test_sm3.hpp"
#include "test_sm4.hpp"
#include "test_snowflake.hpp"
#include "test_uuid.hpp"
#include "test_xxhash.hpp"

int main(int argc, char** argv)
{
    testBase64();
    testMd5();
    testRc4();
    testSha1();
    testSm3();
    testSm4();
    testSnowflake();
    testUUID();
    testXxhash();
    return 0;
}
