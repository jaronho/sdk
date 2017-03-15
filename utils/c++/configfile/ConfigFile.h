/**********************************************************************
* ���ߣ�hezhr
* ʱ�䣺2014-1-3
* �������Զ��������ļ�����xml����ʵ��key-value��ֵ�ԣ�
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
	��	�ܣ���xml�ļ�
	��	����file_name - �ļ�������"asdf.xml"��"../asdf.xml"��"temp\\asdf.xml"��root_name - ���ڵ���
	����ֵ��bool
	*/
	bool open(std::string file_name, std::string root_name = "root");

	/*
	��	�ܣ�����xml�ļ�
	��	����file_name - �ļ�������Ϊ""��Ĭ�ϱ��渲��ԭ�ļ�������"asdf.xml"��"../asdf.xml"��"temp\\asdf.xml"
	����ֵ��bool
	*/
	bool save(std::string file_name = "");

	/*
	��	�ܣ����ýڵ�ֵ
	��	����keyVec - ��ֵ���飻value - ֵ
	����ֵ��bool
	*/
	bool setNodeValue(std::vector<std::string> keyVec, std::string value);

	/*
	��	�ܣ���ȡ�ڵ�ֵ
	��	����keyVec - ��ֵ���飻defaultVal - Ĭ��ֵ
	����ֵ��string
	*/
	std::string getNodeValue(std::vector<std::string> keyVec, std::string defaultVal = "");

	/*
	��	�ܣ����ýڵ�����
	��	����keyVec - ��ֵ���飻attribute - ���ԣ�attributeValue - ����ֵ
	����ֵ��bool
	*/
	bool setNodeAttribute(std::vector<std::string> keyVec, std::string attribute, std::string attributeValue);

	/*
	��	�ܣ���ȡ�ڵ�����
	��	����keyVec - ��ֵ���飻attribute - ���ԣ�defaultVal - Ĭ��ֵ
	����ֵ��string
	*/
	std::string getNodeAttribute(std::vector<std::string> keyVec, std::string attribute, std::string defaultVal = "");


	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// ���湫���ӿ������ڸ��ڵ�Ϊroot���ֵ�һάxml���ݱ��磺
	//<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
	//<root>
	//	<width>640</width>
	//	<height>960</height>
	//</root>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	��	�ܣ�������������
	��	����key - ��ֵ��value - ��������
	����ֵ��bool
	*/
	bool setInt(std::string key, int value);

	/*
	��	�ܣ����õ����ȸ���������
	��	����key - ��ֵ��value - �����ȸ���������
	����ֵ��bool
	*/
	bool setFloat(std::string key, float value);

	/*
	��	�ܣ�����˫���ȸ���������
	��	����key - ��ֵ��value - ˫���ȸ���������
	����ֵ��bool
	*/
	bool setDouble(std::string key, double value);

	/*
	��	�ܣ����ò���������
	��	����key - ��ֵ��value - ����������
	����ֵ��bool
	*/
	bool setBool(std::string key, bool value);

	/*
	��	�ܣ������ַ�������
	��	����key - ��ֵ��value - �ַ�������
	����ֵ��bool
	*/
	bool setString(std::string key, std::string value);

	/*
	��	�ܣ���ȡ��������
	��	����key - ��ֵ��defaultVal - Ĭ��ֵ
	����ֵ��int
	*/
	int getInt(std::string key, int defaultVal = 0);

	/*
	��	�ܣ���ȡ�����ȸ���������
	��	����key - ��ֵ��defaultVal - Ĭ��ֵ
	����ֵ��float
	*/
	float getFloat(std::string key, float defaultVal = 0.0f);

	/*
	��	�ܣ���ȡ˫���ȸ���������
	��	����key - ��ֵ��defaultVal - Ĭ��ֵ
	����ֵ��double
	*/
	double getDouble(std::string key, double defaultVal = 0.0);

	/*
	��	�ܣ���ȡ����������
	��	����key - ��ֵ��defaultVal - Ĭ��ֵ
	����ֵ��bool
	*/
	bool getBool(std::string key, bool defaultVal = false);

	/*
	��	�ܣ���ȡ�ַ�������
	��	����key - ��ֵ��defaultVal - Ĭ��ֵ
	����ֵ��string
	*/
	std::string getString(std::string key, std::string defaultVal = "");

private:
	/*
	��	�ܣ�������xml�ļ�
	��	����file_name - �ļ�������"asdf.xml"��"../asdf.xml"��"temp\\asdf.xml"��root_name - ���ڵ���
	����ֵ��XMLDocument*
	*/
	tinyxml2::XMLDocument* createXmlFile(std::string file_name, std::string root_name = "root");

	/*
	��	�ܣ�����xml�ļ�
	��	����file_name - �ļ�������"asdf.xml"��"../asdf.xml"��"temp\\asdf.xml"
	����ֵ��XMLDocument*
	*/
	tinyxml2::XMLDocument* loadXmlFile(std::string file_name);

	/*
	��	�ܣ�����xml�ַ���
	��	����xml_str - xml�ַ�����length - �ַ�������
	����ֵ��XMLDocument*
	*/
	tinyxml2::XMLDocument* parseXmlFile(const char* xml_str, unsigned long length);

	/*
	��	�ܣ�����xml�ڵ�
	��	����pDoc - XMLDocumentָ�룻keyVec - ��ֵ����
	����ֵ��XMLElement*
	*/
	tinyxml2::XMLElement* createXmlNode(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec);

	/*
	��	�ܣ���ȡxml�ڵ�
	��	����pDoc - XMLDocumentָ�룻keyVec - ��ֵ����
	����ֵ��XMLElement*
	*/
	tinyxml2::XMLElement* getXmlNode(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec);

	/*
	��	�ܣ�����xml�ڵ�ֵ
	��	����pDoc - XMLDocumentָ�룻keyVec - ��ֵ���飻value - �ڵ�ֵ
	����ֵ��bool
	*/
	bool setXmlNodeValue(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec, std::string value);

	/*
	��	�ܣ���ȡxml�ڵ�ֵ
	��	����pDoc - XMLDocumentָ�룻keyVec - ��ֵ����
	����ֵ��string
	*/
	std::string getXmlNodeValue(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec);

	/*
	��	�ܣ�����xml�ڵ�����ֵ
	��	����pDoc - XMLDocumentָ�룻keyVec - ��ֵ���飻attribute - ���ԣ�attributeValue - ����ֵ
	����ֵ��bool
	*/
	bool setXmlNodeAttribute(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec, std::string attribute, std::string attributeValue);

	/*
	��	�ܣ���ȡxml����ֵ
	��	����pDoc - XMLDocumentָ�룻keyVec - ��ֵ���飻attribute - ����
	����ֵ��string
	*/
	std::string getXmlNodeAttribute(tinyxml2::XMLDocument* pDoc, std::vector<std::string> keyVec, std::string attribute);

private:
	std::string mXmlFileName;				// xml�ļ���

	tinyxml2::XMLDocument *mXmlDoc;			// xml�ĵ�ָ��
};


#endif	// _CONFIG_FILE_H_
