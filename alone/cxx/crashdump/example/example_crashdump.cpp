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
    std::string procFile;
#ifdef _WIN32
#else
    char buffer[256];
    getcwd(buffer, sizeof(buffer));
    procFile.append(buffer).append("/");
#endif
    std::string procName(argv[0]);
    size_t pos = procName.find("/");
    if (std::string::npos == pos)
    {
        procFile.append(procName);
    }
    else
    {
        procFile.append(procName.substr(pos + 1, procName.size() - pos));
    }
    std::cout << "====" << procFile << std::endl;
    crashdump::open(procFile, "./dump", [](const std::string& json) { std::cout << json << std::endl; });
    crash2();
    return 0;
}
