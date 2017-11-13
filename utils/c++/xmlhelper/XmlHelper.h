/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-21
* Brief:	xml helper
**********************************************************************/
#ifndef _XML_HELPER_H_
#define _XML_HELPER_H_

#include <string>
#include <vector>
#include "../pugixml/pugixml.hpp"

class XmlHelper {
/**********************************************************************
 **************************** class methods ***************************
 **********************************************************************/
public:
    static std::string toString(pugi::xml_document* doc);

    static bool isXmlFile(const std::string& fileName);

    static bool isXmlString(const std::string& content);

    static pugi::xml_document* createFile(const std::string& fileName, const std::string& rooName = "root");

    static pugi::xml_document* loadFile(const std::string& fileName);

    static pugi::xml_document* loadString(const std::string& content);

    static bool saveFile(pugi::xml_document* doc, const std::string& fileName);

    static bool removeChildren(pugi::xml_node& parent);

    static bool removeNode(pugi::xml_node& parent, const std::string& key);

    static bool removeNode(pugi::xml_node& parent, const std::vector<std::string>& keyVec);

    static pugi::xml_node getNode(pugi::xml_node& parent, const std::string& key, bool createIfNotExist = false);

    static pugi::xml_node getNode(pugi::xml_node& parent, const std::vector<std::string>& keyVec, bool createIfNotExist = false);

    static pugi::xml_text getNodeText(pugi::xml_node& parent, const std::string& key);

    static pugi::xml_text getNodeText(pugi::xml_node& parent, const std::vector<std::string>& keyVec);

    static std::string getNodeValue(pugi::xml_node& parent, const std::string& key, const std::string& defaultValue = "");

    static std::string getNodeValue(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& defaultValue = "");

    static bool setNodeValue(pugi::xml_node& parent, const std::string& key, const std::string& value);

    static bool setNodeValue(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& value);

    static std::string getAttributeValue(pugi::xml_node& parent, const std::string& key, const std::string& attribute, const std::string& defaultValue = "");

    static std::string getAttributeValue(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& attribute, const std::string& defaultValue = "");

    static bool setAttributeValue(pugi::xml_node& parent, const std::string& key, const std::string& attribute, const std::string& attributeValue);

    static bool setAttributeValue(pugi::xml_node& parent, const std::vector<std::string>& keyVec, const std::string& attribute, const std::string& attributeValue);

/**********************************************************************
 ************************** instance methods **************************
 **********************************************************************/
public:
    XmlHelper(void);

    virtual ~XmlHelper(void);

public:
    /*
     * Brief:	get document
     * Param:	void
     * Return:	pugi::xml_document*
     */
    pugi::xml_document* getDocument(void);

    /*
     * Brief:	get root
     * Param:	void
     * Return:	pugi::xml_node
     */
    pugi::xml_node& getRoot(void);

    /*
     * Brief:	get file name
     * Param:	void
     * Return:	string
     */
    std::string getFileName(void);

    /*
     * Brief:	open xml file
     * Param:	fileName - file name, e.g. "asdf.xml"，"../asdf.xml"，"temp\\asdf.xml"
     *			rootName - root name
     * Return:	bool
     */
    bool open(const std::string& fileName, const std::string& rootName = "root");

    /*
     * Brief:	save xml file
     * Param:	fileName - file name, if empty override, or save to new file, e.g. "asdf.xml"，"../asdf.xml"，"temp\\asdf.xml"
     * Return:	bool
     */
    bool save(const std::string& fileName = "");

    /*
     * Brief:	clear keys except root
     * Param:	void
     * Return:	bool
     */
    bool clear(void);

    /*
     * Brief:	convert to xml string
     * Param:	void
     * Return:	std::string
     */
    std::string toString(void);

    /*
     * Brief:	get int value
     * Param:	key - key
     *			defaultValue - default int
     * Return:	int
     */
    int getInt(const std::string& key, int defaultValue = 0);

    /*
     * Brief:	set int value
     * Param:	key - key
     *			value - int
     * Return:	bool
     */
    bool setInt(const std::string& key, int value);

    /*
     * Brief:	get long value
     * Param:	key - key
     *			defaultValue - default long
     * Return:	long
     */
    long getLong(const std::string& key, long defaultValue = 0);

    /*
     * Brief:	set long value
     * Param:	key - key
     *			value - long
     * Return:	bool
     */
    bool setLong(const std::string& key, long value);

    /*
     * Brief:	get float value
     * Param:	key - key
     *			defaultValue - default float
     * Return:	float
     */
    float getFloat(const std::string& key, float defaultValue = 0.0f);

    /*
     * Brief:	set float value
     * Param:	key - key
     *			value - float
     * Return:	bool
     */
    bool setFloat(const std::string& key, float value);

    /*
     * Brief:	get double value
     * Param:	key - key
     *			defaultValue - default double
     * Return:	double
     */
    double getDouble(const std::string& key, double defaultValue = 0.0);

    /*
     * Brief:	set double value
     * Param:	key - key
     *			value - double
     * Return:	bool
     */
    bool setDouble(const std::string& key, double value);

    /*
     * Brief:	get bool value
     * Param:	key - key
     *			defaultValue - default bool
     * Return:	bool
     */
    bool getBool(const std::string& key, bool defaultValue = false);

    /*
     * Brief:	set bool value
     * Param:	key - key
     *			value - bool
     * Return:	bool
     */
    bool setBool(const std::string& key, bool value);

    /*
     * Brief:	get string value
     * Param:	key - key
     *			defaultValue - default string
     * Return:	string
     */
    std::string getString(const std::string& key, const std::string& defaultValue = "");

    /*
     * Brief:	set string value
     * Param:	key - key
     *			value - string
     * Return:	bool
     */
    bool setString(const std::string& key, const std::string& value);

    /*
     * Brief:	remove key
     * Param:	key - key
     * Return:	bool
     */
    bool remove(const std::string& key);

private:
    pugi::xml_document* mDocument;  /* document */
    pugi::xml_node mRoot;           /* root node */
    std::string mFileName;          /* file name */
};

#endif	// _XML_HELPER_H_
