#pragma once

#include <stdio.h>
#include <string.h>
#include <string>
#ifdef _WIN32
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#include "../utilitiy/datetime/datetime.h"

void testDateTime()
{
    printf("\n============================== test datetime =============================\n");

    auto nowDt = utilitiy::DateTime::getNow();
    auto nowTimestamp = utilitiy::DateTime::getNowTimestamp();
    printf("----- [0] now: %04d-%02d-%02d %02d:%02d:%02d.%03d, week day: %d, year day: %d\n", nowDt.year, nowDt.month, nowDt.day,
           nowDt.hour, nowDt.minute, nowDt.second, nowDt.millisecond, nowDt.wday, nowDt.yday);
    printf("----- [0] timestamp: %f\n", nowTimestamp);
    printf("\n");
    auto dt1 = utilitiy::DateTime();
    auto dt2 = nowDt;
    auto dt3 = utilitiy::DateTime(nowTimestamp);
    printf("----- [1] dt1: %04d-%02d-%02d %02d:%02d:%02d.%03d, week day: %d, year day: %d\n", dt1.year, dt1.month, dt1.day, dt1.hour,
           dt1.minute, dt1.second, dt1.millisecond, dt1.wday, dt1.yday);
    printf("----- [1] timestamp1: %f\n", dt1.toTimestamp());
    printf("\n");
    printf("----- [2] dt2: %04d-%02d-%02d %02d:%02d:%02d.%03d, week day: %d, year day: %d\n", dt2.year, dt2.month, dt2.day, dt2.hour,
           dt2.minute, dt2.second, dt2.millisecond, dt2.wday, dt2.yday);
    printf("----- [2] timestamp2: %f\n", dt2.toTimestamp());
    printf("\n");
    printf("----- [3] dt3: %04d-%02d-%02d %02d:%02d:%02d.%03d, week day: %d, year day: %d\n", dt3.year, dt3.month, dt3.day, dt3.hour,
           dt3.minute, dt3.second, dt3.millisecond, dt3.wday, dt3.yday);
    printf("----- [3] timestamp3: %f\n", dt3.toTimestamp());
    printf("\n");
    printf("----- nowDt == dt2, %s\n", (nowDt == dt2 ? "true" : "false"));
    printf("----- nowDt == dt3, %s\n", (nowDt == dt3 ? "true" : "false"));
}
