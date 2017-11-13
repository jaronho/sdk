/**********************************************************************
* Author:	jaron.ho
* Date:		2017-10-18
* Brief:	本地存储
**********************************************************************/
#include "SharePrefs.h"

static XmlHelper* helper = new XmlHelper();
static std::string file_name_bak = "";

static bool copyFile(const std::string& srcFileName, const std::string& destFileName) {
    if (srcFileName.empty() || destFileName.empty() || srcFileName == destFileName) {
        return false;
    }
    FILE* fp_src = fopen(srcFileName.c_str(), "rb");
    if (NULL == fp_src) {
        return false;
    }
	FILE* fp_dest = fopen(destFileName.c_str(), "wb");
	if (NULL == fp_dest) {
		fclose(fp_src);
        return false;
    }
    long fileSize = 0;
    fseek(fp_src, 0, SEEK_END);
    fileSize = ftell(fp_src);
    fseek(fp_src, 0, SEEK_SET);
    unsigned char* fileData = new unsigned char[fileSize];
    fileSize = fread(fileData, sizeof(unsigned char), fileSize, fp_src);
    fclose(fp_src);
    fwrite(fileData, fileSize, sizeof(unsigned char), fp_dest);
	fflush(fp_dest);
    fclose(fp_dest);
    delete fileData;
    return true;
}

bool SharePrefs::open(const std::string& fileName, bool forceReplace /*= true*/, const std::string& fileNameBak /*= ""*/) {
	file_name_bak = fileNameBak;
    if (forceReplace) {
        FILE* fp = fopen(fileName.c_str(), "r");
        if (fp) {
            fclose(fp);
            if (!XmlHelper::isXmlFile(fileName)) {
                ::remove(fileName.c_str());
                if (XmlHelper::isXmlFile(file_name_bak)) {
                    copyFile(file_name_bak, fileName);
                }
            }
        }
    }
    return helper->open(fileName, "root");
}

bool SharePrefs::save(void) {
	copyFile(helper->getFileName(), file_name_bak);
    return helper->save();
}

bool SharePrefs::clear(void) {
    return helper->clear();
}

int SharePrefs::getInt(const std::string& key, int defaultValue /*= 0*/) {
    return helper->getInt(key, defaultValue);
}

bool SharePrefs::setInt(const std::string& key, int value) {
    return helper->setInt(key, value);
}

long SharePrefs::getLong(const std::string& key, long defaultValue /*= 0*/) {
    return helper->getLong(key, defaultValue);
}

bool SharePrefs::setLong(const std::string& key, long value) {
    return helper->setLong(key, value);
}

float SharePrefs::getFloat(const std::string& key, float defaultValue /*= 0.0f*/) {
    return helper->getFloat(key, defaultValue);
}

bool SharePrefs::setFloat(const std::string& key, float value) {
    return helper->setFloat(key, value);
}

double SharePrefs::getDouble(const std::string& key, double defaultValue /*= 0.0*/) {
    return helper->getDouble(key, defaultValue);
}

bool SharePrefs::setDouble(const std::string& key, double value) {
    return helper->setDouble(key, value);
}

bool SharePrefs::getBool(const std::string& key, bool defaultValue /*= false*/) {
    return helper->getBool(key, defaultValue);
}

bool SharePrefs::setBool(const std::string& key, bool value) {
    return helper->setBool(key, value);
}

std::string SharePrefs::getString(const std::string& key, const std::string& defaultValue /*= ""*/) {
    return helper->getString(key, defaultValue);
}

bool SharePrefs::setString(const std::string& key, const std::string& value) {
    return helper->setString(key, value);
}

bool SharePrefs::remove(const std::string& key) {
    return helper->remove(key);
}
