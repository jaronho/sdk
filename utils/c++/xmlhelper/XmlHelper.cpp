/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-21
* Brief:	xml helper
**********************************************************************/
#include "XmlHelper.h"
#include <fstream>

/**********************************************************************
 **************************** class methods ***************************
 **********************************************************************/

pugi::xml_document* XmlHelper::createFile(const std::string& fileName, const std::string& rootName /*= "root"*/) {
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

pugi::xml_document* XmlHelper::loadFile(const std::string& fileName) {
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

pugi::xml_document* XmlHelper::loadString(const std::string& content) {
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

bool XmlHelper::saveFile(pugi::xml_document* doc, const std::string& fileName) {
    if (NULL == doc || fileName.empty()) {
        return false;
    }
    return doc->save_file(fileName.c_str());
}

bool XmlHelper::removeChildren(pugi::xml_node& parent) {
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

bool XmlHelper::removeNode(pugi::xml_node& parent, const std::string& key) {
    if (parent.empty() || key.empty()) {
        return false;
    }
    return parent.remove_child(key.c_str());
}

bool XmlHelper::removeNode(pugi::xml_node& parent, const std::vector<std::string>& keyVec) {
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

pugi::xml_node XmlHelper::getNode(pugi::xml_node& parent, const std::string& key) {
    if (parent.empty() || key.empty()) {
        return pugi::xml_node();
    }
    pugi::xml_node node = parent.child(key.c_str());
    if (node.empty()) {
        node = parent.append_child(key.c_str());
    }
    return node;
}

pugi::xml_node XmlHelper::getNode(pugi::xml_node& parent, const std::vector<std::string>& keyVec) {
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

pugi::xml_text XmlHelper::getNodeText(pugi::xml_node& parent, const std::string& key) {
    return getNode(parent, key).text();
}

pugi::xml_text XmlHelper::getNodeText(pugi::xml_node& parent, const std::vector<std::string>& keyVec) {
    return getNode(parent, keyVec).text();
}

std::string XmlHelper::getNodeValue(pugi::xml_node& parent, const std::string& key, const std::string& defaultValue /*= ""*/) {
    pugi::xml_text text = getNodeText(parent, key);
    if (text.empty()) {
        return defaultValue;
    }
    return text.get();
}

std::string XmlHelper::getNodeValue(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& defaultValue /*= ""*/) {
    pugi::xml_text text = getNodeText(parent, keyVec);
    if (text.empty()) {
        return defaultValue;
    }
    return text.get();
}

bool XmlHelper::setNodeValue(pugi::xml_node& parent, const std::string& key, const std::string& value) {
    pugi::xml_node node = getNode(parent, key);
    if (node.empty()) {
        return false;
    }
    return node.text().set(value.c_str());
}

bool XmlHelper::setNodeValue(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& value) {
    pugi::xml_node node = getNode(parent, keyVec);
    if (node.empty()) {
        return false;
    }
    return node.text().set(value.c_str());
}

std::string XmlHelper::getAttributeValue(pugi::xml_node& parent, const std::string& key, const std::string& attribute, const std::string& defaultValue /*= ""*/) {
    if (parent.empty() || key.empty() || attribute.empty()) {
        return defaultValue;
    }
    pugi::xml_node node = getNode(parent, key);
    if (node.empty()) {
        return defaultValue;
    }
    pugi::xml_attribute attr = node.attribute(attribute.c_str());
    if (attr.empty()) {
        return defaultValue;
    }
    return attr.value();
}

std::string XmlHelper::getAttributeValue(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& attribute, const std::string& defaultValue /*= ""*/) {
    if (parent.empty() || keyVec.empty() || attribute.empty()) {
        return defaultValue;
    }
    pugi::xml_node node = getNode(parent, keyVec);
    if (node.empty()) {
        return defaultValue;
    }
    pugi::xml_attribute attr = node.attribute(attribute.c_str());
    if (attr.empty()) {
        return defaultValue;
    }
    return attr.value();
}

bool XmlHelper::setAttributeValue(pugi::xml_node& parent, const std::string& key, const std::string& attribute, const std::string& attributeValue) {
    if (parent.empty() || key.empty() || attribute.empty()) {
        return false;
    }
    pugi::xml_node node = getNode(parent, key);
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

bool XmlHelper::setAttributeValue(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& attribute, const std::string& attributeValue) {
    if (parent.empty() || keyVec.empty() || attribute.empty()) {
        return false;
    }
    pugi::xml_node node = getNode(parent, keyVec);
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

/**********************************************************************
 ************************** instance methods **************************
 **********************************************************************/

XmlHelper::XmlHelper(void) {
    mDocument = NULL;
    mRoot = pugi::xml_node();
    mFileName = "";
}

XmlHelper::~XmlHelper(void) {
    if (mDocument) {
        delete mDocument;
	}
}

pugi::xml_document* XmlHelper::getDocument(void) {
    return mDocument;
}

pugi::xml_node& XmlHelper::getRoot(void) {
    return mRoot;
}

std::string XmlHelper::getFileName(void) {
    return mFileName;
}

bool XmlHelper::open(const std::string& fileName, const std::string& rootName /*= "root"*/) {
    if (mDocument) {
        delete mDocument;
        mDocument = NULL;
        mRoot = pugi::xml_node();
        mFileName = "";
    }
    if (fileName.empty()) {
        return false;
    }
    FILE* fp = fopen(fileName.c_str(), "r");
    if (NULL == fp) {		/* file is not exist */
        mDocument = createFile(fileName, rootName);
        if (NULL == mDocument) {
            return false;
        }
    } else {				/* file exist */
        fclose(fp);
        mDocument = loadFile(fileName);
        if (NULL == mDocument) {
            return false;
        }
    }
    mRoot = getNode(*mDocument, rootName);
    mFileName = fileName;
    return true;
}

bool XmlHelper::save(const std::string& fileName /*= ""*/) {
    if (NULL == mDocument) {
		return false;
	}
    return saveFile(mDocument, fileName.empty() ? mFileName : fileName);
}

bool XmlHelper::clear(void) {
    if (NULL == mDocument) {
        return false;
    }
    return removeChildren(mRoot);
}

int XmlHelper::getInt(const std::string& key, int defaultValue /*= 0*/) {
    return getNodeText(mRoot, key).as_int(defaultValue);
}

bool XmlHelper::setInt(const std::string& key, int value) {
    char str[32] = {0};
    sprintf(str, "%d", value);
    return setNodeValue(mRoot, key, str);
}

long XmlHelper::getLong(const std::string& key, long defaultValue /*= 0*/) {
    return getNodeText(mRoot, key).as_llong(defaultValue);
}

bool XmlHelper::setLong(const std::string& key, long value) {
    char str[64] = {0};
    sprintf(str, "%ld", value);
    return setNodeValue(mRoot, key, str);
}

float XmlHelper::getFloat(const std::string& key, float defaultValue /*= 0.0f*/) {
    return getNodeText(mRoot, key).as_float(defaultValue);
}

bool XmlHelper::setFloat(const std::string& key, float value) {
    return setDouble(key, (double)value);
}

double XmlHelper::getDouble(const std::string& key, double defaultValue /*= 0.0*/) {
    return getNodeText(mRoot, key).as_double(defaultValue);
}

bool XmlHelper::setDouble(const std::string& key, double value) {
    char str[64] = {0};
    sprintf(str, "%f", value);
    return setNodeValue(mRoot, key, str);
}

bool XmlHelper::getBool(const std::string& key, bool defaultValue /*= false*/) {
    return getNodeText(mRoot, key).as_bool(defaultValue);
}

bool XmlHelper::setBool(const std::string& key, bool value) {
    return setNodeValue(mRoot, key, value ? "true" : "false");
}

std::string XmlHelper::getString(const std::string& key, const std::string& defaultValue /*= ""*/) {
    return getNodeText(mRoot, key).as_string(defaultValue.c_str());
}

bool XmlHelper::setString(const std::string& key, const std::string& value) {
    return setNodeValue(mRoot, key, value);
}

bool XmlHelper::remove(const std::string& key) {
    return removeNode(mRoot, key);
}
