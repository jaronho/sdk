/**********************************************************************
* 作者：hezhr
* 时间：2014-1-3
* 描述：自定义配置文件（在xml表里实现key-value键值对）
**********************************************************************/
#include "ConfigFile.h"
//------------------------------------------------------------------------
ConfigFile::ConfigFile(void) {
	mXmlDoc = NULL;
}
//------------------------------------------------------------------------
ConfigFile::~ConfigFile(void) {
	if (mXmlDoc) {
		delete mXmlDoc;
		mXmlDoc = NULL;
	}
}
//------------------------------------------------------------------------
bool ConfigFile::open(std::string file_name, std::string root_name/* = "root"*/) {
	if (mXmlDoc) {
		delete mXmlDoc;
		mXmlDoc = NULL;
	}
	FILE* fp = fopen(file_name.c_str(), "r");
	if (NULL == fp) {		// 文件不存在
		mXmlDoc = createXmlFile(file_name, root_name);		// 创建xml文件
		if (NULL == mXmlDoc) {
			return false;
		}
	} else {				// 文件存在
		mXmlDoc = loadXmlFile(file_name);					// 加载xml文件
		if (NULL == mXmlDoc) {
			return false;
		}
	}
	mXmlFileName = file_name;
	return true;
}
//------------------------------------------------------------------------
bool ConfigFile::save(std::string file_name /*= ""*/) {
	if (NULL == mXmlDoc) {
		return false;
	}
	if (file_name.empty()) {
		file_name = mXmlFileName;
	}
	FILE* fp = fopen(file_name.c_str(), "w");
	if (NULL == fp) {
		return false;
	}
	tinyxml2::XMLError res = mXmlDoc->SaveFile(fp);
	fclose(fp);
	return tinyxml2::XML_SUCCESS == res;
}
//------------------------------------------------------------------------
bool ConfigFile::setNodeValue(std::vector<std::string> keyVec, std::string value) {
	if (keyVec.empty()) {
		return false;
	}
	return setXmlNodeValue(mXmlDoc, keyVec, value);
}
//------------------------------------------------------------------------
std::string ConfigFile::getNodeValue(std::vector<std::string> keyVec, std::string defaultVal /*= ""*/) {
	if (keyVec.empty()) {
		return defaultVal;
	}
	return getXmlNodeValue(mXmlDoc, keyVec);
}
//------------------------------------------------------------------------
bool ConfigFile::setNodeAttribute(std::vector<std::string> keyVec, std::string attribute, std::string attributeValue) {
	if (keyVec.empty() || attribute.empty()) {
		return false;
	}
	return setXmlNodeAttribute(mXmlDoc, keyVec, attribute, attributeValue);
}
//------------------------------------------------------------------------
std::string ConfigFile::getNodeAttribute(std::vector<std::string> keyVec, std::string attribute, std::string defaultVal/* = ""*/) {
	if (keyVec.empty() || attribute.empty()) {
		return defaultVal;
	}
	return getXmlNodeAttribute(mXmlDoc, keyVec, attribute);
}
//------------------------------------------------------------------------
bool ConfigFile::setInt(std::string key, int value) {
	if (key.empty()) {
		return false;
	}
	std::vector<std::string> keyVec;
	keyVec.push_back("root");
	keyVec.push_back(key);

	char str[64];
	memset(str, 0, sizeof(str));
	sprintf(str, "%d", value);

	return setXmlNodeValue(mXmlDoc, keyVec, str);
}
//------------------------------------------------------------------------
bool ConfigFile::setFloat(std::string key, float value) {
	return setDouble(key, (double)value);
}
//------------------------------------------------------------------------
bool ConfigFile::setDouble(std::string key, double value) {
	if (key.empty()) {
		return false;
	}
	std::vector<std::string> keyVec;
	keyVec.push_back("root");
	keyVec.push_back(key);

	char str[128];
	memset(str, 0, sizeof(str));
	sprintf(str, "%f", value);

	return setXmlNodeValue(mXmlDoc, keyVec, str);
}
//------------------------------------------------------------------------
bool ConfigFile::setBool(std::string key, bool value) {
	if (key.empty()) {
		return false;
	}
	std::vector<std::string> keyVec;
	keyVec.push_back("root");
	keyVec.push_back(key);

	return setXmlNodeValue(mXmlDoc, keyVec, value ? "true" : "false");
}
//------------------------------------------------------------------------
bool ConfigFile::setString(std::string key, std::string value) {
	if (key.empty()) {
		return false;
	}
	std::vector<std::string> keyVec;
	keyVec.push_back("root");
	keyVec.push_back(key);

	return setXmlNodeValue(mXmlDoc, keyVec, value);
}
//------------------------------------------------------------------------
int ConfigFile::getInt(std::string key, int defaultVal /*= 0*/) {
	if (key.empty()) {
		return defaultVal;
	}
	std::vector<std::string> keyVec;
	keyVec.push_back("root");
	keyVec.push_back(key);

	std::string value = getXmlNodeValue(mXmlDoc, keyVec);
	if (value.empty()) {
		return defaultVal;
	}
	return atoi(value.c_str());
}
//------------------------------------------------------------------------
float ConfigFile::getFloat(std::string key, float defaultVal /*= 0.0f*/) {
	return (float)getDouble(key, defaultVal);
}
//------------------------------------------------------------------------
double ConfigFile::getDouble(std::string key, double defaultVal /*= 0.0*/) {
	if (key.empty()) {
		return defaultVal;
	}
	std::vector<std::string> keyVec;
	keyVec.push_back("root");
	keyVec.push_back(key);

	std::string value = getXmlNodeValue(mXmlDoc, keyVec);
	if (value.empty()) {
		return defaultVal;
	}
	return atof(value.c_str());
}
//------------------------------------------------------------------------
bool ConfigFile::getBool(std::string key, bool defaultVal /*= false*/) {
	if (key.empty()) {
		return defaultVal;
	}
	std::vector<std::string> keyVec;
	keyVec.push_back("root");
	keyVec.push_back(key);

	std::string value = getXmlNodeValue(mXmlDoc, keyVec);
	if (value.empty()) {
		return defaultVal;
	}
	return "true" == value;
}
//------------------------------------------------------------------------
std::string ConfigFile::getString(std::string key, std::string defaultVal /*= ""*/) {
	if (key.empty()) {
		return defaultVal;
	}
	std::vector<std::string> keyVec;
	keyVec.push_back("root");
	keyVec.push_back(key);

	return getXmlNodeValue(mXmlDoc, keyVec);
}
//------------------------------------------------------------------------
tinyxml2::XMLDocument* ConfigFile::createXmlFile(std::string file_name, std::string root_name /*= "root"*/) {
	if (file_name.empty()) {
		return NULL;
	}
	tinyxml2::XMLDocument* pDoc = new tinyxml2::XMLDocument();
	if (NULL == pDoc) {
		return NULL;
	}
	const std::string declaration = "xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"";
	tinyxml2::XMLDeclaration* pDeclaration = pDoc->NewDeclaration(declaration.c_str());
	if (NULL == pDeclaration) {
		delete pDoc;
		pDoc = NULL;
		return NULL;
	}
	pDoc->LinkEndChild(pDeclaration);
	tinyxml2::XMLElement* pRootEle = pDoc->NewElement(root_name.c_str());
	if (NULL == pRootEle) {
		delete pDoc;
		pDoc = NULL;
		return NULL;
	}
	pDoc->LinkEndChild(pRootEle);
	if (tinyxml2::XML_SUCCESS == pDoc->SaveFile(file_name.c_str())) {
		return pDoc;
	}
	delete pDoc;
	pDoc = NULL;
	return NULL;
}
//------------------------------------------------------------------------
tinyxml2::XMLDocument* ConfigFile::loadXmlFile(std::string file_name) {
	if (file_name.empty()) {
		return NULL;
	}
	tinyxml2::XMLDocument* pDoc = new tinyxml2::XMLDocument();
	if (tinyxml2::XML_SUCCESS == pDoc->LoadFile(file_name.c_str())) {
		return pDoc;
	}
	delete pDoc;
	pDoc = NULL;
	return NULL;
}
//------------------------------------------------------------------------
tinyxml2::XMLDocument* ConfigFile::parseXmlFile(const char* xml_str, unsigned long length) {
	if (NULL == xml_str || 0 == length) {
		return NULL;
	}
	tinyxml2::XMLDocument* pDoc = new tinyxml2::XMLDocument();
	if (tinyxml2::XML_SUCCESS == pDoc->Parse(xml_str, length)) {
		return pDoc;
	}
	delete pDoc;
	pDoc = NULL;
	return NULL;
}
//------------------------------------------------------------------------
tinyxml2::XMLElement* ConfigFile::createXmlNode(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec) {
	if (NULL == pDoc || keyVec.empty()) {
		return NULL;
	}
	tinyxml2::XMLElement* curNode = pDoc->FirstChildElement();
	if (NULL == curNode) {
		curNode = pDoc->NewElement(keyVec[0].c_str());
		pDoc->LinkEndChild(curNode);
	}

	tinyxml2::XMLElement* node = NULL;
	for (unsigned int i=0; i<keyVec.size(); ++i) {
		std::string key = keyVec[i];
		bool find = false;
		while (curNode) {
			std::string nodeName = curNode->Value();
			if (key == nodeName) {
				node = curNode;
				curNode = curNode->FirstChildElement();
				find = true;
				break;
			}
			curNode = curNode->NextSiblingElement();
		}
		if (!find) {
			curNode = pDoc->NewElement(key.c_str());
			if (NULL == node) {
				pDoc->LinkEndChild(curNode);
			} else {
				node->LinkEndChild(curNode);
			}
			node = curNode;
		}
	}
	return node;
}
//------------------------------------------------------------------------
tinyxml2::XMLElement* ConfigFile::getXmlNode(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec) {
	if (NULL == pDoc || keyVec.empty()) {
		return NULL;
	}
	tinyxml2::XMLElement* curNode = pDoc->FirstChildElement();
	if (NULL == curNode) {
		return NULL;
	}
	tinyxml2::XMLElement* node = NULL;
	for (unsigned int i=0; i<keyVec.size(); ++i) {
		std::string key = keyVec[i];
		bool find = false;
		while (curNode) {
			std::string nodeName = curNode->Value();
			if (key == nodeName) {
				node = curNode;
				curNode = curNode->FirstChildElement();
				find = true;
				break;
			}
			curNode = curNode->NextSiblingElement();
		}
		if (!find) {
			return NULL;
		}
	}
	return node;
}
//------------------------------------------------------------------------
bool ConfigFile::setXmlNodeValue(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec, std::string value) {
	if (NULL == pDoc || keyVec.empty() || value.empty()) {
		return false;
	}
	tinyxml2::XMLElement* node = getXmlNode(pDoc, keyVec);
	if (NULL == node) {
		node = createXmlNode(pDoc, keyVec);
	}
	if (0 == node->GetText()) {
		tinyxml2::XMLText* content = pDoc->NewText(value.c_str());
		node->InsertFirstChild(content);
	} else {
		node->FirstChild()->SetValue(value.c_str());
	}
	return true;
}
//------------------------------------------------------------------------
std::string ConfigFile::getXmlNodeValue(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec) {
	if (NULL == pDoc || keyVec.empty()) {
		return "";
	}
	tinyxml2::XMLElement *node = getXmlNode(pDoc, keyVec);
	if (NULL == node) {
		return "";
	}
	const char* value = node->GetText();
	if (0 == value) {
		return "";
	}
	return value;
}
//------------------------------------------------------------------------
bool ConfigFile::setXmlNodeAttribute(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec, std::string attribute, std::string attributeValue) {
	if (NULL == pDoc || keyVec.empty() || attribute.empty() || attributeValue.empty()) {
		return false;
	}
	tinyxml2::XMLElement* node = getXmlNode(pDoc, keyVec);
	if (NULL == node) {
		node = createXmlNode(pDoc, keyVec);
	}
	node->SetAttribute(attribute.c_str(), attributeValue.c_str());
	return true;
}
//------------------------------------------------------------------------
std::string ConfigFile::getXmlNodeAttribute(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec, std::string attribute) {
	if (NULL == pDoc || keyVec.empty() || attribute.empty()) {
		return "";
	}
	tinyxml2::XMLElement* node = getXmlNode(pDoc, keyVec);
	if (NULL == node) {
		return "";
	}
	const char *att = node->Attribute(attribute.c_str());
	if (0 == att) {
		return "";
	}
	return att;
}
//------------------------------------------------------------------------