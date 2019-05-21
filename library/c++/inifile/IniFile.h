/**********************************************************************
* Author:	jaron.ho
* Date:		2018-04-03
* Brief:	ini file reader/writter
**********************************************************************/
#ifndef _INI_FILE_H_
#define _INI_FILE_H_

#include <string.h>
#include <string>
#include <vector>
#include <map>

struct IniItem {
    std::string key;
    std::string value;
    std::string comment;
};

struct IniSection {
    std::string name;
    std::vector<IniItem> items;
    std::string comment;
};

class IniFile {
public:
    static void trimLeft(std::string& str, char c = ' ');
    static void trimRight(std::string& str, char c = ' ');
    static void trimLeftRightSpace(std::string& str);
    static int getLine(FILE* fp, std::string& line, int lineSize = 128);
    static bool getKeyAndValue(const std::string& content, std::string& key, std::string& value, char c = '=');

public:
    IniFile(void);
    ~IniFile(void);

public:
    /*
     * Brief:	open ini file
     * Param:	fileName - file name, e.g. "asdf.ini","../asdf.ini","temp\\asdf.ini"
     *          lineLength - line length
     *          allowTailComment - whether allow comment at tail of line, e.g. value=1 #abc
     * Return:	int
     *              0.ok
     *              1.fileName is empty
     *              2.can not open file with fileName
     *              3.can not match ']'
     *              4.section is empty
     *              5.duplicate section name
     *              6.parse param failed
     */
    int open(const std::string& fileName, int lineLength = 128, bool allowTailComment = false);

    /*
     * Brief:	save ini file
     * Param:	fileName - file name, e.g. "asdf.ini","../asdf.ini","temp\\asdf.ini"
     * Return:	int
     *              0.ok
     *              1.fileName is empty
     *              2.can not open file with fileName
     */
    int save(const std::string& fileName = "");

    /*
     * Brief:	is exist section
     * Param:	section - section name
     * Return:	bool
     */
    bool hasSection(const std::string& section);

    /*
     * Brief:	is exist key
     * Param:	section - section name
     *          key - key name
     * Return:	bool
     */
    bool hasKey(const std::string& section, const std::string& key);

    /*
     * Brief:	delete section
     * Param:	section - section name
     * Return:	void
     */
    void deleteSection(const std::string& section);

    /*
     * Brief:	delete key
     * Param:	section - section name
     *          key - key name
     * Return:	void
     */
    void deleteKey(const std::string& section, const std::string& key);

    /*
     * Brief:	get comment flags
     * Param:	void
     * Return:	std::vector<std::string>
     */
    std::vector<std::string> getCommentFlags(void);

    /*
     * Brief:	set comment flags
     * Param:	const std::vector<std::string>&
     * Return:	void
     */
    void setCommentFlags(const std::vector<std::string>& flags);

    /*
     * Brief:	get section comment
     * Param:	section - section name
     *          comment - section comment
     * Return:	int
     *              0.ok
     *              1.no exist section
     */
    int getSectionComment(const std::string& section, std::string& comment);

    /*
     * Brief:	set section comment
     * Param:	section - section name
     *          comment - section comment
     * Return:	int
     *              0.ok
     *              1.no exist section
     */
    int setSectionComment(const std::string& section, const std::string& comment);

    /*
     * Brief:	get value
     * Param:	section - section name
     *          key - key name
     *          value - value
     *          comment - section comment
     * Return:	int
     *              0.ok
     *              1.no exist key
     */
    int getValue(const std::string& section, const std::string& key, std::string& value, std::string& comment);

    /*
     * Brief:	set value
     * Param:	section - section name
     *          key - key name
     *          value - value
     *          comment - section comment
     *          forceComment - if force change comment
     * Return:	int
     *              0.ok
     *              1.no exist key
     */
    int setValue(const std::string& section, const std::string& key, const std::string& value, const std::string& comment = "", bool forceComment = false);

    /*
     * Brief:	get value
     * Param:	section - section name
     *          key - key name
     *          value - value
     * Return:	int
     *              0.ok
     *              1.no exist key
     */
    int getValue(const std::string& section, const std::string& key, std::string& value);

    /*
     * Brief:	get int value
     * Param:	section - section name
     *          key - key name
     *          defaultValue - default int
     * Return:	int
     */
    int getInt(const std::string& section, const std::string& key, int defaultValue = 0);

    /*
     * Brief:	set int value
     * Param:	section - section name
     *          key - key name
     *          value - int
     * Return:	void
     */
    void setInt(const std::string& section, const std::string& key, int value);

    /*
     * Brief:	get long value
     * Param:	section - section name
     *          key - key name
     *          defaultValue - default long
     * Return:	long
     */
    long getLong(const std::string& section, const std::string& key, long defaultValue = 0);

    /*
     * Brief:	set long value
     * Param:	section - section name
     *          key - key name
     *          value - long
     * Return:	void
     */
    void setLong(const std::string& section, const std::string& key, long value);

    /*
     * Brief:	get float value
     * Param:	section - section name
     *          key - key name
     *          defaultValue - default float
     * Return:	float
     */
    float getFloat(const std::string& section, const std::string& key, float defaultValue = 0.0f);

    /*
     * Brief:	set float value
     * Param:	section - section name
     *          key - key name
     *          value - float
     * Return:	void
     */
    void setFloat(const std::string& section, const std::string& key, float value);

    /*
     * Brief:	get double value
     * Param:	section - section name
     *          key - key name
     *          defaultValue - default double
     * Return:	double
     */
    double getDouble(const std::string& section, const std::string& key, double defaultValue = 0.0f);

    /*
     * Brief:	set double value
     * Param:	section - section name
     *          key - key name
     *          value - double
     * Return:	void
     */
    void setDouble(const std::string& section, const std::string& key, double value);

    /*
     * Brief:	get bool value
     * Param:	section - section name
     *          key - key name
     *          defaultValue - default bool
     * Return:	bool
     */
    bool getBool(const std::string& section, const std::string& key, bool defaultValue = false);

    /*
     * Brief:	set bool value
     * Param:	section - section name
     *          key - key name
     *          value - bool
     * Return:	void
     */
    void setBool(const std::string& section, const std::string& key, bool value);

    /*
     * Brief:	get string value
     * Param:	section - section name
     *          key - key name
     *          defaultValue - default string
     * Return:	string
     */
    std::string getString(const std::string& section, const std::string& key, const std::string& defaultValue = "");

    /*
     * Brief:	set string value
     * Param:	section - section name
     *          key - key name
     *          value - string
     * Return:	void
     */
    void setString(const std::string& section, const std::string& key, const std::string& value);

private:
    void release(void);
    IniSection* getSection(const std::string& section = "");
    bool isComment(const std::string& str);

private:
    std::string mFileName;      /* ini file name */
    std::map<std::string, IniSection*> mSections;   /* sections */
    std::vector<std::string> mFlags;    /* comment flags */
};

#endif  // _INI_FILE_H_
