#pragma once

#include <stdio.h>
#include <string.h>
#include <string>
#ifdef _WIN32
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#include "../utility/datetime/datetime.h"

void testDateTime()
{
    printf("\n============================== test datetime =============================\n");

    auto nowDt = utility::DateTime::getNow();
    auto nowTimestamp = utility::DateTime::getNowTimestamp();
    printf("----- [0] now: %s.%03d, week day: %d, year day: %d\n", nowDt.yyyyMMddhhmmss().c_str(), nowDt.millisecond, nowDt.wday,
           nowDt.yday);
    printf("----- [0] timestamp: %f\n", nowTimestamp);
    printf("\n");
    auto dt1 = utility::DateTime();
    auto dt2 = nowDt;
    auto dt3 = utility::DateTime(nowTimestamp);
    printf("----- [1] dt1: %s.%03d, week day: %d, year day: %d\n", dt1.yyyyMMddhhmmss().c_str(), dt1.millisecond, dt1.wday, dt1.yday);
    printf("----- [1] timestamp1: %f\n", dt1.toTimestamp());
    printf("\n");
    printf("----- [2] dt2: %s.%03d, week day: %d, year day: %d\n", dt2.yyyyMMddhhmmss().c_str(), dt2.millisecond, dt2.wday, dt2.yday);
    printf("----- [2] timestamp2: %f\n", dt2.toTimestamp());
    printf("\n");
    printf("----- [3] dt3: %s.%03d, week day: %d, year day: %d\n", dt3.yyyyMMddhhmmss("", "", "").c_str(), dt3.millisecond, dt3.wday,
           dt3.yday);
    printf("----- [3] timestamp3: %f\n", dt3.toTimestamp());
    printf("\n");
    printf("----- nowDt == dt2, %s\n", (nowDt == dt2 ? "true" : "false"));
    printf("----- nowDt == dt3, %s\n", (nowDt == dt3 ? "true" : "false"));
}
