/**********************************************************************
* 作者：hezhr
* 时间：2014-1-3
* 描述：自定义配置文件（在xml表里实现key-value键值对）
**********************************************************************/
#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include <string>
#include <vector>
#include "tinyxml2.h"

class ConfigFile {
public:
	ConfigFile(void);

	virtual ~ConfigFile(void);

public:
	/*
	功	能：打开xml文件
	参	数：file_name - 文件名，例"asdf.xml"，"../asdf.xml"，"temp\\asdf.xml"；root_name - 根节点名
	返回值：bool
	*/
	bool open(std::string file_name, std::string root_name = "root");

	/*
	功	能：保存xml文件
	参	数：file_name - 文件名，若为""则默认保存覆盖原文件，例，"asdf.xml"，"../asdf.xml"，"temp\\asdf.xml"
	返回值：bool
	*/
	bool save(std::string file_name = "");

	/*
	功	能：设置节点值
	参	数：keyVec - 键值数组；value - 值
	返回值：bool
	*/
	bool setNodeValue(std::vector<std::string> keyVec, std::string value);

	/*
	功	能：获取节点值
	参	数：keyVec - 键值数组；defaultVal - 默认值
	返回值：string
	*/
	std::string getNodeValue(std::vector<std::string> keyVec, std::string defaultVal = "");

	/*
	功	能：设置节点属性
	参	数：keyVec - 键值数组；attribute - 属性；attributeValue - 属性值
	返回值：bool
	*/
	bool setNodeAttribute(std::vector<std::string> keyVec, std::string attribute, std::string attributeValue);

	/*
	功	能：获取节点属性
	参	数：keyVec - 键值数组；attribute - 属性；defaultVal - 默认值
	返回值：string
	*/
	std::string getNodeAttribute(std::vector<std::string> keyVec, std::string attribute, std::string defaultVal = "");


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 下面公共接口适用于根节点为root名字的一维xml数据表，如：
	//<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	//<root>
	//	<width>640</width>
	//	<height>960</height>
	//</root>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	功	能：设置整型数据
	参	数：key - 键值；value - 整型数据
	返回值：bool
	*/
	bool setInt(std::string key, int value);

	/*
	功	能：设置单精度浮点型数据
	参	数：key - 键值；value - 单精度浮点型数据
	返回值：bool
	*/
	bool setFloat(std::string key, float value);

	/*
	功	能：设置双精度浮点型数据
	参	数：key - 键值；value - 双精度浮点型数据
	返回值：bool
	*/
	bool setDouble(std::string key, double value);

	/*
	功	能：设置布尔型数据
	参	数：key - 键值；value - 布尔型数据
	返回值：bool
	*/
	bool setBool(std::string key, bool value);

	/*
	功	能：设置字符串数据
	参	数：key - 键值；value - 字符串数据
	返回值：bool
	*/
	bool setString(std::string key, std::string value);

	/*
	功	能：获取整型数据
	参	数：key - 键值；defaultVal - 默认值
	返回值：int
	*/
	int getInt(std::string key, int defaultVal = 0);

	/*
	功	能：获取单精度浮点型数据
	参	数：key - 键值；defaultVal - 默认值
	返回值：float
	*/
	float getFloat(std::string key, float defaultVal = 0.0f);

	/*
	功	能：获取双精度浮点型数据
	参	数：key - 键值；defaultVal - 默认值
	返回值：double
	*/
	double getDouble(std::string key, double defaultVal = 0.0);

	/*
	功	能：获取布尔型数据
	参	数：key - 键值；defaultVal - 默认值
	返回值：bool
	*/
	bool getBool(std::string key, bool defaultVal = false);

	/*
	功	能：获取字符串数据
	参	数：key - 键值；defaultVal - 默认值
	返回值：string
	*/
	std::string getString(std::string key, std::string defaultVal = "");

private:
	/*
	功	能：创建空xml文件
	参	数：file_name - 文件名，例"asdf.xml"，"../asdf.xml"，"temp\\asdf.xml"；root_name - 根节点名
	返回值：XMLDocument*
	*/
	tinyxml2::XMLDocument* createXmlFile(std::string file_name, std::string root_name = "root");

	/*
	功	能：加载xml文件
	参	数：file_name - 文件名，例"asdf.xml"，"../asdf.xml"，"temp\\asdf.xml"
	返回值：XMLDocument*
	*/
	tinyxml2::XMLDocument* loadXmlFile(std::string file_name);

	/*
	功	能：解析xml字符串
	参	数：xml_str - xml字符串；length - 字符串长度
	返回值：XMLDocument*
	*/
	tinyxml2::XMLDocument* parseXmlFile(const char* xml_str, unsigned long length);

	/*
	功	能：创建xml节点
	参	数：pDoc - XMLDocument指针；keyVec - 键值数组
	返回值：XMLElement*
	*/
	tinyxml2::XMLElement* createXmlNode(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec);

	/*
	功	能：获取xml节点
	参	数：pDoc - XMLDocument指针；keyVec - 键值数组
	返回值：XMLElement*
	*/
	tinyxml2::XMLElement* getXmlNode(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec);

	/*
	功	能：设置xml节点值
	参	数：pDoc - XMLDocument指针；keyVec - 键值数组；value - 节点值
	返回值：bool
	*/
	bool setXmlNodeValue(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec, std::string value);

	/*
	功	能：获取xml节点值
	参	数：pDoc - XMLDocument指针；keyVec - 键值数组
	返回值：string
	*/
	std::string getXmlNodeValue(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec);

	/*
	功	能：设置xml节点属性值
	参	数：pDoc - XMLDocument指针；keyVec - 键值数组；attribute - 属性；attributeValue - 属性值
	返回值：bool
	*/
	bool setXmlNodeAttribute(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec, std::string attribute, std::string attributeValue);

	/*
	功	能：获取xml属性值
	参	数：pDoc - XMLDocument指针；keyVec - 键值数组；attribute - 属性
	返回值：string
	*/
	std::string getXmlNodeAttribute(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec, std::string attribute);

private:
	std::string mXmlFileName;				// xml文件名

	tinyxml2::XMLDocument *mXmlDoc;			// xml文档指针
};


#endif	// _CONFIG_FILE_H_
