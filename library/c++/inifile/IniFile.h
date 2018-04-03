#ifndef _INI_FILE_H_
#define _INI_FILE_H_

#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
#include<vector>
#include<cstdlib>

struct IniFileEntry {
    IniFileEntry():isComment(false){}
    std::string index;
	std::string name;
	std::string value;
	std::string comment;
    bool isComment;
};

class IniFile {
public:
    IniFile(const char* fileName, bool autoCreate=false);
    ~IniFile(void);
	
public:
	void writeConfigFile(const char* fileName=NULL);
	
    /***********getter*************/
    bool getBoolValue(const char* index, const char* name);
    int getIntValue(const char* index, const char* name);
    float getFloatValue(const char* index, const char* name);
    const char* getStringValue(const char* index, const char* name);
    /***********setter*************/
    void setBoolValue(const char* index, const char* name, bool value);
    void setIntValue(const char* index, const char* name, int value);
    void setFloatValue(const char* index, const char* name, float value);
    void setStringValue(const char *index, const char* name, const char* value);

    /******* for test only *******/
    void printAll();
    
private:
	void loadConfigFile();
	void setStringValueWithIndex(const char* index, const char* name, const char* value);

private:
	std::vector<IniFileEntry> mDatas;
    char str[4096];//for temp string data
    char iniFileName[4096];
    char *data;
	std::fstream* fStream;
    bool autoSave;
    bool autoCreate;
};

#endif	// _INI_FILE_H_