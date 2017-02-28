/**********************************************************************
* 作者：hezhr
* 时间：2012-09-06
* 描述：事件分布
**********************************************************************/
#include "EventDispatcher.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// 所有事件监听者的基类
////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------
Handler::Handler(void)
{
}
//----------------------------------------------------------------------
Handler::~Handler(void)
{
	clearEvents();
}
//----------------------------------------------------------------------
void Handler::handleEvents(int eventId, void* param)
{
}
//----------------------------------------------------------------------
void Handler::addEvent(Event* evt)
{
	if (NULL == evt)
		return;

	if (-1 == getEventIndex(evt))
	{
		mEvents.push_back(evt);
	}
}
//----------------------------------------------------------------------
void Handler::clearEvents(void)
{
	for (size_t i=0; i<mEvents.size(); ++i)
	{
		Event *evt = mEvents[i];
		if (evt)
		{
			evt->removeHandler(this);
		}
	}
	mEvents.clear();
}
//----------------------------------------------------------------------
int Handler::getEventIndex(Event* evt)
{
	for (size_t i=0; i<mEvents.size(); ++i)
	{
		if (evt == mEvents[i])
			return i;
	}
	return -1;
}
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// 事件
////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------
Event::Event(int eventId) : mId(eventId)
{
}
//----------------------------------------------------------------------
Event::~Event(void)
{
	removeAllHandlers();
}
//----------------------------------------------------------------------
int Event::getEventId(void)
{
	return mId;
}
//----------------------------------------------------------------------
void Event::addHandler(Handler* hd)
{
	if (NULL == hd)
		return;

	if (-1 == getHandlerIndex(hd))
	{
		mHandlers.push_back(hd);
		hd->addEvent(this);
	}
}
//----------------------------------------------------------------------
void Event::addHandler(CallFunc callfunc)
{
	if (NULL == callfunc)
		return;

	if (-1 == getCallFuncIndex(callfunc))
	{
		mCallFuncs.push_back(callfunc);
	}
}
//----------------------------------------------------------------------
void Event::removeHandler(Handler* hd)
{
	if (NULL == hd)
		return;

	int index = getHandlerIndex(hd);
	if (-1 == index)
		return;

	mHandlers.erase(mHandlers.begin() + index);
}
//----------------------------------------------------------------------
void Event::removeHandler(CallFunc callfunc)
{
	if (NULL == callfunc)
		return;

	int index = getCallFuncIndex(callfunc);
	if (-1 == index)
		return;

	mCallFuncs.erase(mCallFuncs.begin() + index);
}
//----------------------------------------------------------------------
void Event::removeAllHandlers(void)
{
	mHandlers.clear();
	mCallFuncs.clear();
}
//----------------------------------------------------------------------
void Event::excute(void* param /* = NULL */)
{
	for (size_t i=0; i<mHandlers.size(); ++i)
	{
		Handler *hdl = mHandlers[i];
		if (hdl)
		{
			hdl->handleEvents(mId, param);
		}
	}
	//
	for (size_t j=0; j<mCallFuncs.size(); ++j)
	{
		CallFunc cf = mCallFuncs[j];
		if (cf)
		{
			cf(param);
		}
	}
}
//----------------------------------------------------------------------
int Event::getHandlerIndex(Handler* hd)
{
	for (size_t i=0; i<mHandlers.size(); ++i)
	{
		if (hd == mHandlers[i])
			return i;
	}
	return -1;
}
//----------------------------------------------------------------------
int Event::getCallFuncIndex(CallFunc callfunc)
{
	for (size_t i=0; i<mCallFuncs.size(); ++i)
	{
		if (callfunc == mCallFuncs[i])
			return i;
	}
	return -1;
}
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// 事件派发器
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::map<int, Event*>::iterator				DISPATCHER_ITER;
//----------------------------------------------------------------------
EventDispatcher::EventDispatcher(void)
{
}
//----------------------------------------------------------------------
EventDispatcher::~EventDispatcher(void)
{
	clear();
}
//----------------------------------------------------------------------
void EventDispatcher::subscribe(int eventId, Handler* hd)
{
	if (NULL == hd)
		return;
	
	if (mEventMap.end() == mEventMap.find(eventId))
	{
		mEventMap.insert(std::make_pair(eventId, new Event(eventId)));
	}
	mEventMap[eventId]->addHandler(hd);
}
//----------------------------------------------------------------------
void EventDispatcher::subscribe(int eventId, CallFunc callfunc)
{
	if (NULL == callfunc)
		return;

	if (mEventMap.end() == mEventMap.find(eventId))
	{
		mEventMap.insert(std::make_pair(eventId, new Event(eventId)));
	}
	mEventMap[eventId]->addHandler(callfunc);
}
//----------------------------------------------------------------------
void EventDispatcher::unsubscribe(int eventId, Handler* hd)
{
	if (mEventMap.end() != mEventMap.find(eventId))
	{
		mEventMap[eventId]->removeHandler(hd);
	}
}
//----------------------------------------------------------------------
void EventDispatcher::unsubscribe(int eventId, CallFunc callfunc)
{
	if (mEventMap.end() != mEventMap.find(eventId))
	{
		mEventMap[eventId]->removeHandler(callfunc);
	}
}
//----------------------------------------------------------------------
void EventDispatcher::unsubscribe(int eventId)
{
	DISPATCHER_ITER iter = mEventMap.find(eventId);
	if (mEventMap.end() != iter)
	{
		delete (iter->second);
		iter->second = NULL;
		mEventMap.erase(iter);
	}
}
//----------------------------------------------------------------------
void EventDispatcher::post(int eventId, void* param /* = NULL */)
{
	if (mEventMap.end() != mEventMap.find(eventId))
	{
		mEventMap[eventId]->excute(param);
	}
}
//----------------------------------------------------------------------
void EventDispatcher::clear(void)
{
	DISPATCHER_ITER iter = mEventMap.begin();
	for (; mEventMap.end() != iter; ++iter)
	{
		delete (iter->second);
		iter->second = NULL;
	}
	mEventMap.clear();
}
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// 数据派发器
////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------
DataDispatcher::DataDispatcher(void)
{
}
//----------------------------------------------------------------------
DataDispatcher::~DataDispatcher(void)
{
	mIntMap.clear();
	mUnsignedIntMap.clear();
	mFloatMap.clear();
	mDoubleMap.clear();
	mCharMap.clear();
	mStringMap.clear();
	mBoolMap.clear();
}
//----------------------------------------------------------------------
void DataDispatcher::setInt(int dataId, int data)
{
	if (mIntMap.end() == mIntMap.find(dataId))
	{
		mIntMap.insert(std::make_pair(dataId, data));
	}
	mIntMap[dataId] = data;
}
//----------------------------------------------------------------------
void DataDispatcher::setUnsignedInt(int dataId, unsigned int data)
{
	if (mUnsignedIntMap.end() == mUnsignedIntMap.find(dataId))
	{
		mUnsignedIntMap.insert(std::make_pair(dataId, data));
	}
	mUnsignedIntMap[dataId] = data;
}
//----------------------------------------------------------------------
void DataDispatcher::setFloat(int dataId, float data)
{
	if (mFloatMap.end() == mFloatMap.find(dataId))
	{
		mFloatMap.insert(std::make_pair(dataId, data));
	}
	mFloatMap[dataId] = data;
}
//----------------------------------------------------------------------
void DataDispatcher::setDouble(int dataId, double data)
{
	if (mDoubleMap.end() == mDoubleMap.find(dataId))
	{
		mDoubleMap.insert(std::make_pair(dataId, data));
	}
	mDoubleMap[dataId] = data;
}
//----------------------------------------------------------------------
void DataDispatcher::setChar(int dataId, char data)
{
	if (mCharMap.end() == mCharMap.find(dataId))
	{
		mCharMap.insert(std::make_pair(dataId, data));
	}
	mCharMap[dataId] = data;
}
//----------------------------------------------------------------------
void DataDispatcher::setString(int dataId, std::string data)
{
	if (mStringMap.end() == mStringMap.find(dataId))
	{
		mStringMap.insert(std::make_pair(dataId, data));
	}
	mStringMap[dataId] = data;
}
//----------------------------------------------------------------------
void DataDispatcher::setBool(int dataId, bool data)
{
	if (mBoolMap.end() == mBoolMap.find(dataId))
	{
		mBoolMap.insert(std::make_pair(dataId, data));
	}
	mBoolMap[dataId] = data;
}
//----------------------------------------------------------------------
int DataDispatcher::getInt(int dataId)
{
	if (mIntMap.end() == mIntMap.find(dataId))
		return 0;
	
	return mIntMap[dataId];
}
//----------------------------------------------------------------------
unsigned int DataDispatcher::getUnsignedInt(int dataId)
{
	if (mUnsignedIntMap.end() == mUnsignedIntMap.find(dataId))
		return 0;
	
	return mUnsignedIntMap[dataId];
}
//----------------------------------------------------------------------
float DataDispatcher::getFloat(int dataId)
{
	if (mFloatMap.end() == mFloatMap.find(dataId))
		return 0.0f;
	
	return mFloatMap[dataId];
}
//----------------------------------------------------------------------
double DataDispatcher::getDouble(int dataId)
{
	if (mDoubleMap.end() == mDoubleMap.find(dataId))
		return 0.0f;
	
	return mDoubleMap[dataId];
}
//----------------------------------------------------------------------
char DataDispatcher::getChar(int dataId)
{
	if (mCharMap.end() == mCharMap.find(dataId))
		return '\0';
	
	return mCharMap[dataId];
}
//----------------------------------------------------------------------
std::string DataDispatcher::getString(int dataId)
{
	if (mStringMap.end() == mStringMap.find(dataId))
		return "";
	
	return mStringMap[dataId];
}
//----------------------------------------------------------------------
bool DataDispatcher::getBool(int dataId)
{
	if (mBoolMap.end() == mBoolMap.find(dataId))
		return false;
	
	return mBoolMap[dataId];
}
//----------------------------------------------------------------------


