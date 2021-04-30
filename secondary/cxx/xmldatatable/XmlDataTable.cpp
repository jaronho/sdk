/**********************************************************************
* 作者：hezhr
* 时间：2012-9-9
* 描述：xml数据表（保存从外部导入的xml数据），生成一张二维表
**********************************************************************/
#include "XmlDataTable.h"
#include <algorithm>


//------------------------------------------------------------------------
XmlDataTable::XmlDataTable(void)
{
}
//------------------------------------------------------------------------
XmlDataTable::~XmlDataTable(void)
{
	clear();
}
// ------------------------------------------------------------------------
bool XmlDataTable::load(const char* file_name)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError res = doc.LoadFile(file_name);
	if (tinyxml2::XML_SUCCESS == res)
		return createTable(doc);
	
	return false;
}
//------------------------------------------------------------------------
bool XmlDataTable::parse(const char* xml_str, unsigned long length)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError res = doc.Parse(xml_str, length);
	if (tinyxml2::XML_SUCCESS == res)
		return createTable(doc);
	
	return false;
}
//------------------------------------------------------------------------
bool XmlDataTable::createTable(tinyxml2::XMLDocument& doc)
{
	tinyxml2::XMLElement *root = doc.RootElement();
	if (NULL == root)
		return false;

	clear();
	// 行信息
	for (tinyxml2::XMLElement *row = root->FirstChildElement(); row; row = row->NextSiblingElement())
	{
		TColumns *col_strs = new TColumns(mCols.size());
		// 列信息
		for (tinyxml2::XMLElement *col = row->FirstChildElement(); col; col = col->NextSiblingElement())
		{
			const char *col_name = col->Value();
			const char *text = col->GetText();
			const char *col_value = NULL == text ? "" : text;
			// 
			TColumns::iterator begin = mCols.begin();
			TColumns::iterator end = mCols.end();
			TColumns::iterator iter = std::find(begin, end, col_name);
			if (end == iter)	// 列名不存在,插入列
			{
				mCols.push_back(col_name);
				col_strs->push_back(col_value);
			}
			else				// 列名已存在,填充内容
			{
				(*col_strs)[iter - begin] = col_value;
			}
		}
		mRows.push_back(col_strs);
	}
	// 调整每行的列数
	unsigned int count = mCols.size();
	TRows::iterator iter = mRows.begin();
	TRows::iterator end = mRows.end();
	for (iter; end != iter; ++iter)
	{
		if (count != (*iter)->size())
		{
			(*iter)->resize(count);
		}
	}
	return true;
}
//------------------------------------------------------------------------
const char* XmlDataTable::cell(unsigned int row, unsigned int col) const
{
	if (false == existsCell(row, col))
		return NULL;

	return (*mRows[row])[col].c_str();
}
//------------------------------------------------------------------------
const char* XmlDataTable::cell(unsigned int row, const char* col_name) const
{
	if (false == existsCell(row, col_name))
		return NULL;
	
	return (*mRows[row])[getColIndex(col_name)].c_str();
}
//------------------------------------------------------------------------
bool XmlDataTable::existsCell(unsigned int row, unsigned int col) const
{
	return existsRow(row) && existsCol(col);
}
//------------------------------------------------------------------------
bool XmlDataTable::existsCell(unsigned int row, const char* col_name) const
{
	return existsRow(row) && existsCol(col_name);
}
//------------------------------------------------------------------------
bool XmlDataTable::existsRow(unsigned int index) const
{
	return index < mRows.size();
}
//------------------------------------------------------------------------
bool XmlDataTable::existsCol(unsigned int index) const
{
	return index < mCols.size();
}
//------------------------------------------------------------------------
const char* XmlDataTable::getColName(unsigned int col_index) const
{
	if (col_index >= mCols.size())
		return NULL;

	return mCols[col_index].c_str();
}
//------------------------------------------------------------------------
bool XmlDataTable::existsCol(const char* col_name) const
{
	TColumns::const_iterator begin = mCols.begin();
	TColumns::const_iterator end = mCols.end();
	TColumns::const_iterator iter = std::find(begin, end, col_name);
	return  end != iter;
}
//------------------------------------------------------------------------
int XmlDataTable::getColIndex(const char* col_name) const
{
	TColumns::const_iterator begin = mCols.begin();
	TColumns::const_iterator end = mCols.end();
	TColumns::const_iterator iter = std::find(begin, end, col_name);
	if (end == iter)
		return -1;
	
	return iter - begin;			
}
//------------------------------------------------------------------------
unsigned int XmlDataTable::getRowCount(void) const
{
	return mRows.size();
}
//------------------------------------------------------------------------
unsigned int XmlDataTable::getColCount(void) const
{
	return mCols.size();
}
//------------------------------------------------------------------------
bool XmlDataTable::isEmpty(void) const
{
	return mRows.empty();
}
//------------------------------------------------------------------------
void XmlDataTable::clear(void)
{
	mCols.clear();
	TRows::iterator iter = mRows.begin();
	TRows::iterator end = mRows.end();
	for (iter; end != iter; ++iter)
	{
		delete *iter;
		*iter = NULL;
	}
	mRows.clear();
}
//------------------------------------------------------------------------

