#include "IniFile.h"
#include <sstream>
#include <map>

#define IniFile_DEBUG 0
#define log printf
#define mlog(msg...) do{\
if(IniFile_DEBUG) printf(msg);\
}while(0)

IniFile::IniFile(const char* fileNameWithPath, bool _autoCreate):
    data(NULL),
    fStream(NULL),
    autoSave(false),
    autoCreate(_autoCreate)
{
    strcpy(iniFileName, fileNameWithPath);
    loadConfigFile();
}

void IniFile::loadConfigFile()
{
	std::fstream fStream;
	std::string p = iniFileName;

	fStream.open(p, std::ios::in);
    if(!fStream){
        if(!autoCreate){
            log("load config, file [%s] not exist", iniFileName);
        }else{
            log("inifile [%s] not found, create a new file", iniFileName);
        }
        return;
    }else{
        mlog("file open OK\n");
    }
    char line[4096];
    char ch;
    int i=0;
	std::string index;
	std::string str;
    bool isComment=false;
    while(!fStream.eof()){
        fStream.read(&ch, 1);

        IniFileEntry entry;
        if(ch=='#' && i==0) isComment = true;
        if(isComment==true && (ch=='\n' || ch=='\r')) {
            isComment=false;
            line[i++]='\0';
            i=0;
            entry.isComment = true;
            entry.comment = line;
			mDatas.push_back(entry);
        }
        if(isComment==true) {
            line[i++]=ch;
            continue;
        }
        //zfu: all up for comment
        if(ch != '\n' || ch=='\r') line[i++]=ch;
        else{
            if(i==0) continue;
            line[i]='\0';
            str = std::string(line);
            mlog("read one line {%s}", str.c_str());
            if(line[0]=='['){
                index = str;
            }else{
                entry.index = index.substr(1,index.length()-2);
                int fIndex = str.find_first_of('=');
                entry.name = str.substr(0,fIndex);
                entry.value = str.substr(fIndex+1, str.length()-fIndex-1);
				mDatas.push_back(entry);
                mlog("entry: index@[%s]\t name@[%s]\t value@[%s]", entry.index.c_str(), entry.name.c_str(), entry.value.c_str());
            }
            i=0;
        }
    }
    if(i!=0) {
        IniFileEntry entry;
        entry.index = str;
        int fIndex = str.find_first_of('=');
        entry.name = str.substr(0,fIndex);
        entry.value = str.substr(fIndex+1, str.length()-fIndex-1);
		mDatas.push_back(entry);
        mlog("last add entry: index@[%s]\t name@[%s]\t value@[%s]", entry.index.c_str(), entry.name.c_str(), entry.value.c_str());
    }
    fStream.close();
}

IniFile::~IniFile()
{
    if(autoSave){
		std::cout<<"AUTO save Config file["<<iniFileName<<"]"<< std::endl;
        writeConfigFile();
    }
}

void IniFile::writeConfigFile(const char* fileName)
{
    autoSave=false;
    if(fileName==NULL) fileName = iniFileName;
	std::fstream fStream;
    fStream.open(fileName, std::ios_base::out | std::ios_base::trunc);
    mlog("start write file[%s]", fileName);
	std::string index = std::string("");
    bool withComment = false;
    bool isStart=true;
    for(std::vector<IniFileEntry>::iterator it= mDatas.begin(); it!= mDatas.end(); it++){
        IniFileEntry entry = *it;
        if(entry.isComment) {
            withComment=true;
            if(isStart) fStream<<entry.comment.c_str()<< std::endl;
            else fStream<< std::endl<<entry.comment.c_str()<< std::endl;
            isStart=false;
            
            mlog("write comment:%s", entry.comment.c_str());
            continue;
        }
        if(strcmp(index.c_str(), entry.index.c_str()) != 0){
            index = entry.index;
            if(withComment || isStart) {
                fStream<<'['<<entry.index<<']'<< std::endl;
                withComment=false; isStart=false;
                mlog("write index[%s]", entry.index.c_str());
            }
            else{
                fStream<<endl<<'['<<entry.index<<']'<<endl;
                mlog("write index [%s]", entry.index.c_str());
            }
        }
        if (strlen(entry.name.c_str())==0 || strlen(entry.value.c_str())==0) {
            mlog("skip invalid entry");
            continue;
        }
        fStream<<entry.name<<'='<<entry.value<< std::endl;
        mlog("write :%s=%s", entry.name.c_str(), entry.value.c_str());
    }
    fStream<< std::endl;
    fStream.close();
    mlog("write configfile[%s] end", fileName);
}

void IniFile::setStringValueWithIndex(const char *index, const char* name, const char* value)
{
    autoSave = true;
    IniFileEntry entry;
    entry.index = index;
    entry.name = name;
    entry.value = value;
    if(mDatas.size() == 0) {/*cout<<"data is NULL, push and return"<<endl; */
		mDatas.push_back(entry);
        return;
    }
	std::vector<IniFileEntry>::iterator it= mDatas.begin();
    bool findIndex=false;
    bool findName=false;
	std::vector<IniFileEntry>::iterator itInsertPos;
    for(it= mDatas.begin(); it!= mDatas.end(); it++){
        if(findIndex==false){
            if(strcmp(it->index.c_str(), index) == 0){
                findIndex=true;
            }
        }
        if(findIndex==true){
            if(strcmp(it->index.c_str(), index) != 0){
                break;
            }else{
                itInsertPos = it;
            }
            if(strcmp(it->name.c_str(), name)==0){
                findName=true;
                itInsertPos = it;
                break;
            }
            continue;
        }
        itInsertPos=it;
    }
    if(findIndex && findName){
        itInsertPos->value = std::string(value);
        return;
    }

	mDatas.insert(++itInsertPos, 1, entry);
}

/***********getter*************/
bool IniFile::getBoolValue(const char* index, const char *name)
{
    const char *str = getStringValue(index, name);
    if(str == NULL) {log("notfound for [%s]-[%s]", index, name); return false;}
    if(strcmp(str,"true") == 0) return true;
    else return false;
}

int IniFile::getIntValue(const char *index, const char* name)
{
    const char *str = getStringValue(index, name);
    if(!str){
        return -1;
    }else{
        return atoi(str);
    }
}

float IniFile::getFloatValue(const char* index, const char *name)
{
    const char *str = getStringValue(index, name);
    if(str == NULL) { std::cout<<"notfound"<< std::endl; return -1.0;}
    return atof(str);
}

const char* IniFile::getStringValue(const char* index, const char *name)
{
    mlog("find index[%s]-name[%s]", index, name);
    for(unsigned int i=0; i<mDatas.size(); i++){
        if(strcmp(mDatas[i].index.c_str(), index) == 0){
            mlog("find index[%s]", mDatas[i].index.c_str());
            for(;i<mDatas.size();i++){
                if(strcmp(mDatas[i].name.c_str(), name)==0)
                    return mDatas[i].value.c_str();
            }
        }
    }
    cout<<"DEBUG: ["<<index<<"] of--["<<name<<"] not found"<<endl;
    return NULL;
}

/***********setter*************/
void IniFile::setBoolValue(const char* index, const char *name, bool value)
{
    if(value) sprintf(str, "true");
    else sprintf(str, "false");
    setStringValueWithIndex(index,name,str);
}

void IniFile::setIntValue(const char* index, const char *name, int value)
{
    sprintf(str, "%d", value);
    setStringValueWithIndex(index,name,str);
}

void IniFile::setFloatValue(const char* index, const char *name, float value)
{
    sprintf(str, "%f", value);
    setStringValueWithIndex(index,name,str);
}

void IniFile::setStringValue(const char *index, const char* name, const char* value)
{
    setStringValueWithIndex(index,name,value);
}

void IniFile::printAll() {
    for (std::vector<IniFileEntry>::iterator it = mDatas.begin(); mDatas.end() != it; ++it){
        IniFileEntry entry = *it;
        log("--------print All Entry of file[%s]------------", iniFileName);
        if (entry.isComment) {
			std::cout << entry.comment << std::endl;
			continue;
		}
		printf_s("  index:%s", entry.index.c_str());
        log("  index:%s", entry.index.c_str());
        log("  name:%s", entry.name.c_str());
        log("  value:%s", entry.value.c_str());
    }
}