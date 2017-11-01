/**********************************************************************
* Author:	jaron.ho
* Date:		2017-10-18
* Brief:	本地存储
**********************************************************************/
#include "SharePrefs.h"

static XmlHelper* helper = new XmlHelper();

bool SharePrefs::open(const std::string& fileName, bool forceReplace /*= true*/) {
    if (forceReplace) {
        FILE* fp = fopen(fileName.c_str(), "r");
        if (fp) {
            fclose(fp);
            if (!XmlHelper::isXmlFile(fileName)) {
                ::remove(fileName.c_str());
            }
        }
    }
    return helper->open(fileName, "root");
}

bool SharePrefs::save(void) {
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
