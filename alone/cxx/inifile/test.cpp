#include <iostream>
#include "IniFile.h"

using namespace std;

int main()
{
    cout << "Hello World!" << endl;
    IniFile ini;
    int ret = ini.open("test.ini");
    ini.setString("SERVER", "IP", "190.168.3.96");
    ini.setInt("SERVER", "PORT", 5967);
    ini.setString("HTTP", "POST", "http://www.baidu.com");
    ini.setBool("HTTP", "AUTO", true);
    ini.save();
    std::string ip = ini.getString("SERVER", "IP");
    int port = ini.getInt("SERVER", "PORT");
    printf("%s === %d\n", ip.c_str(), port);
    return 0;
}