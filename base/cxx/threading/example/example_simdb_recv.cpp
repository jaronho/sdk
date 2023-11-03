#include <chrono>
#include <iostream>
#include <thread>

#include "threading/simdb.hpp"

int main()
{
    simdb db("simdb_test", 1024, 2048);
    while (1)
    {
        size_t num = 0;
        db.get("num", &num, sizeof(num));
        printf("recv: %lld\n", num);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
