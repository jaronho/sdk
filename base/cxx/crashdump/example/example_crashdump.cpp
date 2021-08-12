#include <iostream>
#include <string.h>
#ifdef _WIN32
#else
#include <unistd.h>
#endif
#include "crashdump/crashdump.h"

class Person
{
public:
    std::string name;
    int age;
};

void crash1()
{
    volatile int* a = (int*)(NULL);
    *a = 1;
}

void crash2()
{
    Person* p = new Person();
    delete p;
    p = nullptr;
    p->name = "Jim";
}

int main(int argc, char** argv)
{
    crashdump::start("./dump", [](const std::string& json) { std::cout << json << std::endl; });
    crashdump::setProcVersion("1.0.0");
    crash2();
    return 0;
}
