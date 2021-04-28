/******************************************************************************
* 作者: hezhr
* 时间: 2013-11-6
* 描述: 逻辑相关的表格
******************************************************************************/
#include "LogicTable.h"
#include "../CommonFunc/CommonFunc.h"
#include "cocos2d.h"

using namespace cocos2d;

//-----------------------------------------------------------------------
XmlDataTable LogicTableMgr::createXmlDataTable(const std::string& filename)
{
	XmlDataTable table;
	unsigned long length;
	char *buffer = (char*)(CCFileUtils::sharedFileUtils()->getFileData(filename.c_str(), "rb", &length));
	if (NULL == buffer)
		return table;

	std::string str(buffer);
	str[length] = '\0';		// 注意这里设置结束符
	delete buffer;
	buffer = NULL;

	table.parse(str.c_str(), str.size());
	return table;
}
//-----------------------------------------------------------------------
void LogicTableMgr::load()
{
	loadDailyLoginRewardTable();
	loadBigWinnerRewardTable();
	loadCorpseFlowerRewardTable();
	loadLineRewardTable();
	loadScoreRewardTable();
}
//-----------------------------------------------
void LogicTableMgr::loadDailyLoginRewardTable()
{
	XmlDataTable table = createXmlDataTable("DailyLoginRewardTable.xml");
	// 获取列id
	const size_t colDays		= table.getColIndex("days");
	const size_t colReward		= table.getColIndex("reward");
	// 顺序读取数据保存到内存中
	for (size_t i=0; i<table.getRowCount(); ++i)
	{
		DailyLoginRewardRow row;
		row.days		= CommonFunc::toInt(table.cell(i, colDays));
		row.reward		= CommonFunc::toInt(table.cell(i, colReward));

		if (mDailyLoginRewardTable.end() != mDailyLoginRewardTable.find(row.days))
			continue;

		if (0 == row.days)
			continue;

		mDailyLoginRewardTable.insert(std::make_pair(row.days, row));
	}
}
//-----------------------------------------------------------------------
const DailyLoginRewardRow* LogicTableMgr::getDailyLoginRewardRow(size_t days) const
{
	DailyLoginRewardTable::const_iterator iter = mDailyLoginRewardTable.find(days);
	if (mDailyLoginRewardTable.end() == iter)
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableMgr::loadBigWinnerRewardTable()
{
	XmlDataTable table = createXmlDataTable("BigWinnerRewardTable.xml");
	// 获取列id
	const size_t colCount		= table.getColIndex("count");
	const size_t colReward		= table.getColIndex("reward");
	// 顺序读取数据保存到内存中
	for (size_t i=0; i<table.getRowCount(); ++i)
	{
		BigWinnerRewardRow row;
		row.count		= CommonFunc::toInt(table.cell(i, colCount));
		row.reward		= CommonFunc::toInt(table.cell(i, colReward));

		if (mBigWinnerRewardTable.end() != mBigWinnerRewardTable.find(row.count))
			continue;

		if (0 == row.count)
			continue;

		mBigWinnerRewardTable.insert(std::make_pair(row.count, row));
	}
}
//-----------------------------------------------------------------------
const BigWinnerRewardRow* LogicTableMgr::getBigWinnerRewardRow(size_t count) const
{
	BigWinnerRewardTable::const_iterator iter = mBigWinnerRewardTable.find(count);
	if (mBigWinnerRewardTable.end() == iter)
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableMgr::loadCorpseFlowerRewardTable()
{
	XmlDataTable table = createXmlDataTable("CorpseFlowerRewardTable.xml");
	// 获取列id
	const size_t colNum			= table.getColIndex("num");
	const size_t colReward		= table.getColIndex("reward");
	// 顺序读取数据保存到内存中
	for (size_t i=0; i<table.getRowCount(); ++i)
	{
		CorpseFlowerRewardRow row;
		row.num		= CommonFunc::toInt(table.cell(i, colNum));
		row.reward	= CommonFunc::toInt(table.cell(i, colReward));

		if (mCorpseFlowerRewardTable.end() != mCorpseFlowerRewardTable.find(row.num))
			continue;

		if (0 == row.num)
			continue;

		mCorpseFlowerRewardTable.insert(std::make_pair(row.num, row));
	}
}
//-----------------------------------------------------------------------
const CorpseFlowerRewardRow* LogicTableMgr::getCorpseFlowerRewardRow(size_t num) const
{
	CorpseFlowerRewardTable::const_iterator iter = mCorpseFlowerRewardTable.find(num);
	if (mCorpseFlowerRewardTable.end() == iter)
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableMgr::loadLineRewardTable()
{
	XmlDataTable table = createXmlDataTable("LineRewardTable.xml");
	// 获取列id
	const size_t colLines		= table.getColIndex("lines");
	const size_t colReward		= table.getColIndex("reward");
	// 顺序读取数据保存到内存中
	for (size_t i=0; i<table.getRowCount(); ++i)
	{
		LineRewardRow row;
		row.lines		= CommonFunc::toInt(table.cell(i, colLines));
		row.reward		= CommonFunc::toInt(table.cell(i, colReward));

		if (mLineRewardTable.end() != mLineRewardTable.find(row.lines))
			continue;

		if (0 == row.lines)
			continue;

		mLineRewardTable.insert(std::make_pair(row.lines, row));
	}
}
//-----------------------------------------------------------------------
const LineRewardRow* LogicTableMgr::getLineRewardRow(size_t lines) const
{
	LineRewardTable::const_iterator iter = mLineRewardTable.find(lines);
	if (mLineRewardTable.end() == iter)
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------
void LogicTableMgr::loadScoreRewardTable()
{
	XmlDataTable table = createXmlDataTable("ScoreRewardTable.xml");
	// 获取列id
	const size_t colScores		= table.getColIndex("scores");
	const size_t colReward		= table.getColIndex("reward");
	// 顺序读取数据保存到内存中
	for (size_t i=0; i<table.getRowCount(); ++i)
	{
		ScoreRewardRow row;
		row.scores		= CommonFunc::toInt(table.cell(i, colScores));
		row.reward		= CommonFunc::toInt(table.cell(i, colReward));

		if (mScoreRewardTable.end() != mScoreRewardTable.find(row.scores))
			continue;

		if (0 == row.scores)
			continue;

		mScoreRewardTable.insert(std::make_pair(row.scores, row));
	}
}
//-----------------------------------------------------------------------
const ScoreRewardRow* LogicTableMgr::getScoreRewardRow(size_t scores) const
{
	ScoreRewardTable::const_iterator iter = mScoreRewardTable.find(scores);
	if (mScoreRewardTable.end() == iter)
		return NULL;

	return &(iter->second);
}
//-----------------------------------------------------------------------

