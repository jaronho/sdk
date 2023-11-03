#include <chrono>
#include <iostream>
#include <thread>

#include "ipc/simdb/simdb.hpp"

int main()
{
    simdb db("simdb_test", 1024, 2048);
    size_t num = 0;
    while (1)
    {
        if (num > 999999999999999999)
        {
            num = 0;
        }
        ++num;
        db.put("num", &num, sizeof(num));
        printf("send: %lld\n", num);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}
