/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-21
* Brief:	xml helper
**********************************************************************/
#include "XmlHelper.h"
#include <memory.h>
#include <fstream>
#include <unistd.h>

/**********************************************************************
 **************************** class methods ***************************
 **********************************************************************/

pugi::xml_document* XmlHelper::createXmlFile(const std::string& fileName, const std::string& rootName /*= "root"*/) {
    if (fileName.empty()) {
        return NULL;
    }
    pugi::xml_document* doc = new pugi::xml_document();
    if (NULL == doc) {
        return NULL;
    }
    if (!doc->append_child(rootName.empty() ? "root" : rootName.c_str())) {
        delete doc;
        return NULL;
    }
    if (!doc->save_file(fileName.c_str())) {
        delete doc;
        return NULL;
    }
    return doc;
}

pugi::xml_document* XmlHelper::loadXmlFile(const std::string& fileName) {
    if (fileName.empty()) {
        return NULL;
    }
    pugi::xml_document* doc = new pugi::xml_document();
    if (NULL == doc) {
        delete doc;
        return NULL;
    }
    pugi::xml_parse_result result = doc->load_file(fileName.c_str());
    if (pugi::status_ok != result.status) {
        delete doc;
        return NULL;
    }
    return doc;
}

pugi::xml_document* XmlHelper::loadXmlString(const std::string& content) {
    if (content.empty()) {
        return NULL;
    }
    pugi::xml_document* doc = new pugi::xml_document();
    if (NULL == doc) {
        delete doc;
        return NULL;
    }
    pugi::xml_parse_result result = doc->load_string(content.c_str());
    if (pugi::status_ok != result.status) {
        delete doc;
        return NULL;
    }
    return doc;
}

bool XmlHelper::saveXmlFile(pugi::xml_document* doc, const std::string& fileName) {
    if (NULL == doc || fileName.empty()) {
        return false;
    }
    return doc->save_file(fileName.c_str());
}

bool XmlHelper::removeXmlChildren(pugi::xml_node& parent) {
    if (parent.empty()) {
        return false;
    }
    pugi::xml_node node = parent.first_child();
    while (!node.empty()) {
        parent.remove_child(node);
        node = parent.first_child();
    }
    return true;
}

bool XmlHelper::removeXmlNode(pugi::xml_node& parent, const std::string& key) {
    if (parent.empty() || key.empty()) {
        return false;
    }
    return parent.remove_child(key.c_str());
}

bool XmlHelper::removeXmlNode(pugi::xml_node& parent, const std::vector<std::string>& keyVec) {
    if (parent.empty() || keyVec.empty()) {
        return false;
    }
    pugi::xml_node parentTmp = parent;
    pugi::xml_node node;
    for (unsigned int i = 0, len = keyVec.size(); i < len; ++i) {
        const std::string& key = keyVec[i];
        node = parentTmp.child(key.c_str());
        if (node.empty()) {
            return false;
        }
        parentTmp = node;
    }
    return node.parent().remove_child(node);
}

pugi::xml_node XmlHelper::getXmlNode(pugi::xml_node& parent, const std::string& key) {
    if (parent.empty() || key.empty()) {
        return pugi::xml_node();
    }
    pugi::xml_node node = parent.child(key.c_str());
    if (node.empty()) {
        node = parent.append_child(key.c_str());
    }
    return node;
}

pugi::xml_node XmlHelper::getXmlNode(pugi::xml_node& parent, const std::vector<std::string>& keyVec) {
    if (parent.empty() || keyVec.empty()) {
        return pugi::xml_node();
    }
    pugi::xml_node parentTmp = parent;
    pugi::xml_node node;
    for (unsigned int i = 0, len = keyVec.size(); i < len; ++i) {
        const std::string& key = keyVec[i];
        node = parentTmp.child(key.c_str());
        if (node.empty()) {
            node = parentTmp.append_child(key.c_str());
            if (node.empty()) {
                return pugi::xml_node();
            }
        }
        parentTmp = node;
    }
    return node;
}

pugi::xml_text XmlHelper::getXmlNodeText(pugi::xml_node& parent, const std::string& key) {
    return getXmlNode(parent, key).text();
}

pugi::xml_text XmlHelper::getXmlNodeText(pugi::xml_node& parent, const std::vector<std::string>& keyVec) {
    return getXmlNode(parent, keyVec).text();
}

bool XmlHelper::setXmlNodeValue(pugi::xml_node& parent, const std::string& key, const std::string& value) {
    if (parent.empty() || key.empty() || value.empty()) {
        return false;
    }
    pugi::xml_node node = getXmlNode(parent, key);
    if (node.empty()) {
        return false;
    }
    return node.text().set(value.c_str());
}

bool XmlHelper::setXmlNodeValue(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& value) {
    if (parent.empty() || keyVec.empty() || value.empty()) {
        return false;
    }
    pugi::xml_node node = getXmlNode(parent, keyVec);
    if (node.empty()) {
        return false;
    }
    return node.text().set(value.c_str());
}

std::string XmlHelper::getXmlNodeValue(pugi::xml_node& parent, const std::string& key, const std::string& defaultValue /*= ""*/) {
    if (parent.empty() || key.empty()) {
        return defaultValue;
    }
    pugi::xml_node node = getXmlNode(parent, key);
    if (node.empty()) {
        return defaultValue;
    }
    return node.text().get();
}

std::string XmlHelper::getXmlNodeValue(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& defaultValue /*= ""*/) {
    if (parent.empty() || keyVec.empty()) {
        return defaultValue;
    }
    pugi::xml_node node = getXmlNode(parent, keyVec);
    if (node.empty()) {
        return defaultValue;
    }
    return node.text().get();
}

bool XmlHelper::setXmlNodeAttribute(pugi::xml_node& parent, const std::string& key, const std::string& attribute, const std::string& attributeValue) {
    if (parent.empty() || key.empty() || attribute.empty() || attributeValue.empty()) {
        return false;
    }
    pugi::xml_node node = getXmlNode(parent, key);
    if (node.empty()) {
        return false;
    }
    pugi::xml_attribute attr = node.attribute(attribute.c_str());
    if (attr.empty()) {
        attr = node.append_attribute(attribute.c_str());
        if (attr.empty()) {
            return false;
        }
    }
    return attr.set_value(attributeValue.c_str());
}

bool XmlHelper::setXmlNodeAttribute(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& attribute, const std::string& attributeValue) {
    if (parent.empty() || keyVec.empty() || attribute.empty() || attributeValue.empty()) {
        return false;
    }
    pugi::xml_node node = getXmlNode(parent, keyVec);
    if (node.empty()) {
        return false;
    }
    pugi::xml_attribute attr = node.attribute(attribute.c_str());
    if (attr.empty()) {
        attr = node.append_attribute(attribute.c_str());
        if (attr.empty()) {
            return false;
        }
    }
    return attr.set_value(attributeValue.c_str());
}

std::string XmlHelper::getXmlNodeAttribute(pugi::xml_node& parent, const std::string& key, const std::string& attribute, const std::string& defaultValue /*= ""*/) {
    if (parent.empty() || key.empty() || attribute.empty()) {
        return defaultValue;
    }
    pugi::xml_node node = getXmlNode(parent, key);
    if (node.empty()) {
        return defaultValue;
    }
    pugi::xml_attribute attr = node.attribute(attribute.c_str());
    if (attr.empty()) {
        return defaultValue;
    }
    return attr.value();
}

std::string XmlHelper::getXmlNodeAttribute(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& attribute, const std::string& defaultValue /*= ""*/) {
    if (parent.empty() || keyVec.empty() || attribute.empty()) {
        return defaultValue;
    }
    pugi::xml_node node = getXmlNode(parent, keyVec);
    if (node.empty()) {
        return defaultValue;
    }
    pugi::xml_attribute attr = node.attribute(attribute.c_str());
    if (attr.empty()) {
        return defaultValue;
    }
    return attr.value();
}

/**********************************************************************
 ************************** instance methods **************************
 **********************************************************************/

XmlHelper::XmlHelper(void) {
    mXmlDoc = NULL;
    mXmlRoot = pugi::xml_node();
    mXmlFileName = "";
}

XmlHelper::~XmlHelper(void) {
	if (mXmlDoc) {
        delete mXmlDoc;
	}
}

pugi::xml_document* XmlHelper::getDocument(void) {
    return mXmlDoc;
}

pugi::xml_node& XmlHelper::getRoot(void) {
    return mXmlRoot;
}

std::string XmlHelper::getFileName(void) {
    return mXmlFileName;
}

bool XmlHelper::open(const std::string& fileName, const std::string& rootName /*= "root"*/) {
    if (mXmlDoc) {
        delete mXmlDoc;
        mXmlDoc = NULL;
        mXmlRoot = pugi::xml_node();
        mXmlFileName = "";
    }
    if (fileName.empty()) {
        return false;
    }
    FILE* fp = fopen(fileName.c_str(), "r");
    if (NULL == fp) {		/* file is not exist */
        mXmlDoc = createXmlFile(fileName, rootName);
        if (NULL == mXmlDoc) {
            return false;
        }
    } else {				/* file exist */
        fclose(fp);
        mXmlDoc = loadXmlFile(fileName);
        if (NULL == mXmlDoc) {
            return false;
        }
    }
    mXmlRoot = getXmlNode(*mXmlDoc, rootName);
    mXmlFileName = fileName;
    return true;
}

bool XmlHelper::save(const std::string& fileName /*= ""*/) {
	if (NULL == mXmlDoc) {
		return false;
	}
    return saveXmlFile(mXmlDoc, fileName.empty() ? mXmlFileName : fileName);
}

bool XmlHelper::clear(void) {
    if (NULL == mXmlDoc) {
        return false;
    }
    return removeXmlChildren(mXmlRoot);
}

int XmlHelper::getInt(const std::string& key, int defaultValue /*= 0*/) {
    return getXmlNodeText(mXmlRoot, key).as_int(defaultValue);
}

bool XmlHelper::setInt(const std::string& key, int value) {
    char str[32];
    memset(str, 0, sizeof(str));
    sprintf(str, "%d", value);
    return setXmlNodeValue(mXmlRoot, key, str);
}

long XmlHelper::getLong(const std::string& key, long defaultValue /*= 0*/) {
    return getXmlNodeText(mXmlRoot, key).as_llong(defaultValue);
}

bool XmlHelper::setLong(const std::string& key, long value) {
    char str[64];
    memset(str, 0, sizeof(str));
    sprintf(str, "%ld", value);
    return setXmlNodeValue(mXmlRoot, key, str);
}

float XmlHelper::getFloat(const std::string& key, float defaultValue /*= 0.0f*/) {
    return getXmlNodeText(mXmlRoot, key).as_float(defaultValue);
}

bool XmlHelper::setFloat(const std::string& key, float value) {
    return setDouble(key, (double)value);
}

double XmlHelper::getDouble(const std::string& key, double defaultValue /*= 0.0*/) {
    return getXmlNodeText(mXmlRoot, key).as_double(defaultValue);
}

bool XmlHelper::setDouble(const std::string& key, double value) {
    char str[64];
    memset(str, 0, sizeof(str));
    sprintf(str, "%f", value);
    return setXmlNodeValue(mXmlRoot, key, str);
}

bool XmlHelper::getBool(const std::string& key, bool defaultValue /*= false*/) {
    return getXmlNodeText(mXmlRoot, key).as_bool(defaultValue);
}

bool XmlHelper::setBool(const std::string& key, bool value) {
    return setXmlNodeValue(mXmlRoot, key, value ? "true" : "false");
}

std::string XmlHelper::getString(const std::string& key, const std::string& defaultValue /*= ""*/) {
    return getXmlNodeText(mXmlRoot, key).as_string(defaultValue.c_str());
}

bool XmlHelper::setString(const std::string& key, const std::string& value) {
    return setXmlNodeValue(mXmlRoot, key, value);
}

bool XmlHelper::remove(const std::string& key) {
    return removeXmlNode(mXmlRoot, key);
}
