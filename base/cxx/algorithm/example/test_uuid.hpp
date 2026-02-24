#pragma once

#include <stdio.h>
#include <string.h>
#include <string>

#include "../algorithm/uuid/uuid.h"

void testUUID()
{
    printf("\n============================== test uuid =============================\n");
    auto uuid = algorithm::UUID::generateUUIDv7();
    auto uuidStr = algorithm::UUID::uuidToString(uuid);
    printf("UUID[0]: %s\n", uuidStr.c_str());
    printf("UUID[1]: %s\n", algorithm::UUID::generateUUIDv7String().c_str());
    printf("UUID[2]: %s\n", algorithm::UUID::generateUUIDv7String().c_str());
}
