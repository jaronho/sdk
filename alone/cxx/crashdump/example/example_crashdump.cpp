#include "crashdump/crashdump.h"

#include <iostream>
#include <string.h>
#ifdef _WIN32
#else
#include <unistd.h>
#endif

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
    p->name = "jim";
}

int main(int argc, char** argv)
{
    std::string fullProcName;
#ifdef _WIN32
#else
    char buffer[256];
    getcwd(buffer, sizeof(buffer));
    fullProcName.append(buffer).append("/");
#endif
    std::string procName(argv[0]);
    size_t pos = procName.find("/");
    if (std::string::npos == pos)
    {
        fullProcName.append(procName);
    }
    else
    {
        fullProcName.append(procName.substr(pos + 1, procName.size() - pos));
    }
    std::cout << "====" << fullProcName << std::endl;
    crashdump::open("./dump", fullProcName,
                    [](const std::string& dumpFilename) { std::cout << "dumpFilename: " << dumpFilename << std::endl; });
    crash2();
    return 0;
}
