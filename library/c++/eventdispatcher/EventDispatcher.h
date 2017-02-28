/**********************************************************************
* 作者：hezhr
* 时间：2012-09-06
* 描述：事件分布
**********************************************************************/
#ifndef _EVENT_DISPATCHER_H_
#define _EVENT_DISPATCHER_H_

#include <string>
#include <vector>
#include <map>

class Handler;
class Event;
class EventDispatcher;

typedef void (*CallFunc)(void*);
#define EventCallFunc(callfunc) (CallFunc)(&callfunc)



/*
说	明：所有事件监听者的基类
*/
class Handler
{
public:
	Handler(void);

	~Handler(void);

public:
	/*
	功	能：处理所监听的事件，若子类要处理事件，则需要重写此函数
	参	数：eventId - 事件id；param - 参数
	返回值：void
	*/
	virtual void handleEvents(int eventId, void* param);

	/*
	功	能：添加监听事件
	参	数：evt - 参数
	返回值：void
	*/
	void addEvent(Event* evt);

private:
	/*
	功	能：清除所有监听事件
	参	数：void
	返回值：void
	*/
	void clearEvents(void);

	/*
	功	能：获取监听事件索引
	参	数：evt - 参数
	返回值：void
	*/
	int getEventIndex(Event* evt);

private:
	std::vector<Event*> mEvents;			// 句柄所监听的事件集
};



/*
说	明：事件
*/
class Event
{
public:
	Event(int eventId);

	~Event(void);
	
public:
	/*
	功	能：获取事件标识
	参	数：void
	返回值：int
	*/
	int getEventId(void);
	
	/*
	功	能：添加事件处理句柄
	参	数：hd - 事件处理句柄
	返回值：void
	*/
	void addHandler(Handler* hd);

	/*
	功	能：添加事件处理函数
	参	数：callfunc - 事件处理函数
	返回值：void
	*/
	void addHandler(CallFunc callfunc);
	
	/*
	功	能：移除事件处理句柄
	参	数：hd - 事件处理句柄
	返回值：void
	*/
	void removeHandler(Handler* hd);

	/*
	功	能：移除事件处理函数
	参	数：callfunc - 事件处理函数
	返回值：void
	*/
	void removeHandler(CallFunc callfunc);
	
	/*
	功	能：移除所有事件处理
	参	数：void
	返回值：void
	*/
	void removeAllHandlers(void);
	
	/*
	功	能：执行事件
	参	数：param - 参数
	返回值：void
	*/
	void excute(void* param = NULL);

private:
	/*
	功	能：获取事件处理句柄索引
	参	数：hd - 事件处理句柄
	返回值：int
	*/
	int getHandlerIndex(Handler* hd);

	/*
	功	能：获取事件处理函数索引
	参	数：callfunc - 事件处理函数
	返回值：int
	*/
	int getCallFuncIndex(CallFunc callfunc);
	
private:
	int mId;									// 事件标识

	std::vector<Handler*> mHandlers;			// 事件处理句柄

	std::vector<CallFunc> mCallFuncs;			// 事件处理函数
};



/*
说	明：事件派发器
*/
class EventDispatcher
{
public:
	EventDispatcher(void);

	~EventDispatcher(void);
	
public:
	/*
	功	能：注册事件
	参	数：eventId - 事件标识；hd - 事件处理句柄
	返回值：void
	*/
	void subscribe(int eventId, Handler* hd);

	/*
	功	能：注册事件
	参	数：eventId - 事件标识；callfunc - 事件处理函数
	返回值：void
	*/
	void subscribe(int eventId, CallFunc callfunc);
	
	/*
	功	能：取消事件注册
	参	数：eventId - 事件标识；hd - 要取消注册的事件处理句柄
	返回值：void
	*/
	void unsubscribe(int eventId, Handler* hd);

	/*
	功	能：取消事件注册
	参	数：eventId - 事件标识；callfunc - 要取消注册的事件处理函数
	返回值：void
	*/
	void unsubscribe(int eventId, CallFunc callfunc);
	
	/*
	功	能：取消事件注册
	参	数：eventId - 事件标识
	返回值：void
	*/
	void unsubscribe(int eventId);
	
	/*
	功	能：发布事件
	参	数：eventId - 事件标识；param - 参数
	返回值：void
	*/
	void post(int eventId, void* param = NULL);
	
	/*
	功	能：清除所有事件
	参	数：void
	返回值：void
	*/
	void clear(void);
	
private:
	std::map<int, Event*> mEventMap;		// 事件集
};



/*
说	明：数据派发器
*/
class DataDispatcher
{
public:
	DataDispatcher(void);

	~DataDispatcher(void);
	
public:
	/*
	功	能：设置整型数据
	参	数：dataId - 数据标识；data - 整型数据
	返回值：void
	*/
	void setInt(int dataId, int data);

	/*
	功	能：设置无符号整型数据
	参	数：dataId - 数据标识；data - 无符号整型数据
	返回值：void
	*/
	void setUnsignedInt(int dataId, unsigned int data);

	/*
	功	能：设置单精度浮点型数据
	参	数：dataId - 数据标识；data - 单精度浮点型数据
	返回值：void
	*/
	void setFloat(int dataId, float data);

	/*
	功	能：设置双精度浮点型数据
	参	数：dataId - 数据标识；data - 双精度浮点型数据
	返回值：void
	*/
	void setDouble(int dataId, double data);

	/*
	功	能：设置单字符数据
	参	数：dataId - 数据标识；data - 单字符数据
	返回值：void
	*/
	void setChar(int dataId, char data);

	/*
	功	能：设置字符串数据
	参	数：dataId - 数据标识；data - 字符串数据
	返回值：void
	*/
	void setString(int dataId, std::string data);

	/*
	功	能：设置布尔型数据
	参	数：dataId - 数据标识；data - 布尔型数据
	返回值：void
	*/
	void setBool(int dataId, bool data);

	/*
	功	能：获取整型数据
	参	数：dataId - 数据标识
	返回值：int
	*/
	int getInt(int dataId);

	/*
	功	能：获取无符号整型数据
	参	数：dataId - 数据标识
	返回值：unsigned int
	*/
	unsigned int getUnsignedInt(int dataId);

	/*
	功	能：获取单精度浮点型数据
	参	数：dataId - 数据标识
	返回值：float
	*/
	float getFloat(int dataId);

	/*
	功	能：获取双精度浮点型数据
	参	数：dataId - 数据标识
	返回值：double
	*/
	double getDouble(int dataId);

	/*
	功	能：获取单字符数据
	参	数：dataId - 数据标识
	返回值：char
	*/
	char getChar(int dataId);

	/*
	功	能：获取字符串数据
	参	数：dataId - 数据标识
	返回值：std::string
	*/
	std::string getString(int dataId);

	/*
	功	能：获取布尔型数据
	参	数：dataId - 数据标识
	返回值：bool
	*/
	bool getBool(int dataId);

private:
	std::map<int, int> mIntMap;						// 整型集

	std::map<int, unsigned int> mUnsignedIntMap;	// 无符号整型集

	std::map<int, float> mFloatMap;					// 单精度浮点型集

	std::map<int, double> mDoubleMap;				// 双精度浮点型集

	std::map<int, char> mCharMap;					// 单字符集

	std::map<int, std::string> mStringMap;			// 字符串集

	std::map<int, bool> mBoolMap;					// 布尔型集
};


#endif	// _EVENT_DISPATCHER_H_


