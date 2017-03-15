/**********************************************************************
* 作者：hezhr
* 时间：2012-9-9
* 描述：xml数据表（保存从外部导入的xml数据），生成一张二维表
**********************************************************************/
#ifndef _XML_DATA_TABLE_H_
#define _XML_DATA_TABLE_H_

#include <string>
#include <vector>
#if defined WIN32 || defined ANDROID
#include "support\tinyxml2\tinyxml2.h"
#else
#include "tinyxml2.h"
#endif


class XmlDataTable
{
public:
	typedef std::vector<std::string> TColumns;		// 列类型
	typedef std::vector<TColumns*> TRows;			// 行类型

public:
	XmlDataTable(void);
	virtual ~XmlDataTable(void);

public:
	/*
	功	能：读取xml文件
	参	数：file_name - 文件名，包含后缀名，可以包含相对路径，不能是绝对路径，例，"asdf.xml"，"../asdf.xml"，"temp\\asdf.xml"
	返回值：bool
	*/
	bool load(const char* file_name);

	/*
	功	能：解析xml字符串
	参	数：xml_str - xml字符串；length - 字符串长度
	返回值：bool
	*/
	bool parse(const char* xml_str, unsigned long length);
	
	/*
	功	能：获取某个单元内容
	参	数：row - 行号；col - 列号
	返回值：const char*
	*/
	const char* cell(unsigned int row, unsigned int col) const;

	/*
	功	能：获取某个单元内容
	参	数：row - 行号；col_name - 列名
	返回值：const char*
	*/
	const char* cell(unsigned int row, const char* col_name) const;

	/*
	功	能：是否存在某个单元
	参	数：row - 行号；col - 列号
	返回值：bool
	*/
	bool existsCell(unsigned int row, unsigned int col) const;

	/*
	功	能：是否存在某个单元
	参	数：row - 行号；col_name - 列名
	返回值：bool
	*/
	bool existsCell(unsigned int row, const char* col_name) const;

	/*
	功	能：是否存在某行
	参	数：row - 行号
	返回值：bool
	*/
	bool existsRow(unsigned int row) const;

	/*
	功	能：是否存在某列
	参	数：bool -列号
	返回值：bool
	*/
	bool existsCol(unsigned int col) const;

	/*
	功	能：根据列索引获取列名
	参	数：col_index - 列索引
	返回值：const char*
	*/
	const char* getColName(unsigned int col_index) const;

	/*
	功	能：是否存在某列
	参	数：col_name - 列名
	返回值：bool
	*/
	bool existsCol(const char* col_name) const;

	/*
	功	能：根据列名获取列索引
	参	数：col_name - 列名
	返回值：int
	*/
	int getColIndex(const char* col_name) const;

	/*
	功	能：获取行数
	参	数：void
	返回值：unsigned int
	*/
	unsigned int getRowCount(void) const;

	/*
	功	能：获取列数
	参	数：void
	返回值：unsigned int
	*/
	unsigned int getColCount(void) const;

	/*
	功	能：xml表是否为空
	参	数：void
	返回值：bool
	*/
	bool isEmpty(void) const;

private:
	/*
	功	能：生成二维表
	参	数：XMLDocument - 类
	返回值：bool
	*/
	bool createTable(tinyxml2::XMLDocument& doc);

	/*
	功	能：清除
	参	数：void
	返回值：void
	*/
	void clear(void);

private:
	TColumns mCols;				// 列内容
	TRows mRows;				// 行内容
};


#endif	// _XML_DATA_TABLE_H_

