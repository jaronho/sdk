/**********************************************************************
* 作者：hezhr
* 时间：2013-09-16
* 描述：事件中心
**********************************************************************/
#ifndef _EVENT_CENTER_H_
#define _EVENT_CENTER_H_

#include "EventDispatcher.h"


/*
说	明：事件中心
*/
class EventCenter
{
public:
	static void subscribe(int evt, Handler* hd);
	
	static void subscribe(int evt, CallFunc callfunc);
	
	static void unsubscribe(int evt, Handler* hd);
	
	static void unsubscribe(int evt, CallFunc callfunc);
	
	static void unsubscribe(int evt);
	
	static void post(int evt, void* param = NULL);

private:
	static void init(void);
	
private:
	static EventDispatcher *mEventDispatcher;
};



/*
说	明：数据中心
*/
class DataCenter
{
public:
	static void setInt(int dataId, int data);

	static void setUnsignedInt(int dataId, unsigned int data);

	static void setFloat(int dataId, float data);

	static void setDouble(int dataId, double data);

	static void setChar(int dataId, char data);

	static void setString(int dataId, std::string data);

	static void setBool(int dataId, bool data);

	static int getInt(int dataId);
	
	static unsigned int getUnsignedInt(int dataId);

	static float getFloat(int dataId);

	static double getDouble(int dataId);

	static char getChar(int dataId);

	static std::string getString(int dataId);

	static bool getBool(int dataId);

private:
	static void init(void);

private:
	static DataDispatcher *mDataDispatcher;
};


#endif	// _EVENT_CENTER_H_


