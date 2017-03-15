/******************************************************************************
* 作者: hezhr
* 时间: 2013-11-6
* 描述: 逻辑相关的表格
******************************************************************************/
#ifndef _LOGIC_TABLE_H_
#define _LOGIC_TABLE_H_

#include <string>
#include <map>
#include "XmlDataTable.h"


// 每日登陆奖励表
struct DailyLoginRewardRow
{
	size_t days;			// 天数
	size_t reward;			// 奖励
};
typedef std::map<size_t, DailyLoginRewardRow> DailyLoginRewardTable;

// 大玩家奖励表
struct BigWinnerRewardRow
{
	size_t count;			// 胜利次数
	size_t reward;			// 奖励
};
typedef std::map<size_t, BigWinnerRewardRow> BigWinnerRewardTable;

// 消除食人花奖励表
struct CorpseFlowerRewardRow
{
	size_t num;				// 消除的食人花数量
	size_t reward;			// 奖励
};
typedef std::map<size_t, CorpseFlowerRewardRow> CorpseFlowerRewardTable;

// 消除行数奖励表
struct LineRewardRow
{
	size_t lines;			// 消除的行数
	size_t reward;			// 奖励
};
typedef std::map<size_t, LineRewardRow> LineRewardTable;

// 得分奖励表
struct ScoreRewardRow
{
	size_t scores;			// 得分
	size_t reward;			// 奖励
};
typedef std::map<size_t, ScoreRewardRow> ScoreRewardTable;



////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// 数据表管理器
class LogicTableMgr
{
public:
	// 单件
	static LogicTableMgr* getInstance(void)
	{
		static LogicTableMgr* instance = NULL;
		if (NULL == instance)
		{
			instance = new LogicTableMgr();
		}
		return instance;
	}

public:
	void load();	// 加载所有数据表
	// 获取二维表数据
	const DailyLoginRewardRow* getDailyLoginRewardRow(size_t days) const;
	const DailyLoginRewardTable& getDailyLoginRewardTable(void) const { return mDailyLoginRewardTable; }

	const BigWinnerRewardRow* getBigWinnerRewardRow(size_t count) const;
	const BigWinnerRewardTable& getBigWinnerRewardTable(void) const { return mBigWinnerRewardTable; }

	const CorpseFlowerRewardRow* getCorpseFlowerRewardRow(size_t num) const;
	const CorpseFlowerRewardTable& getCorpseFlowerRewardTable(void) const { return mCorpseFlowerRewardTable; }

	const LineRewardRow* getLineRewardRow(size_t lines) const;
	const LineRewardTable& getLineRewardTable(void) const { return mLineRewardTable; }

	const ScoreRewardRow* getScoreRewardRow(size_t scores) const;
	const ScoreRewardTable& getScoreRewardTable(void) const { return mScoreRewardTable; }

private:
	XmlDataTable createXmlDataTable(const std::string& filename);	// 根据文件名创建二维表
	// 加载二维表
	void loadDailyLoginRewardTable(void);
	void loadBigWinnerRewardTable(void);
	void loadCorpseFlowerRewardTable(void);
	void loadLineRewardTable(void);
	void loadScoreRewardTable(void);

private:
	DailyLoginRewardTable mDailyLoginRewardTable;
	BigWinnerRewardTable mBigWinnerRewardTable;
	CorpseFlowerRewardTable mCorpseFlowerRewardTable;
	LineRewardTable mLineRewardTable;
	ScoreRewardTable mScoreRewardTable;
};

#define deLogicTableMgr	LogicTableMgr::getInstance()


#endif	// _LOGIC_TABLE_H_


