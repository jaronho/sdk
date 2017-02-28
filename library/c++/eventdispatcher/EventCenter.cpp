/**********************************************************************
* 作者：hezhr
* 时间：2013-09-16
* 描述：事件中心
**********************************************************************/
#include "EventCenter.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// 事件中心
////////////////////////////////////////////////////////////////////////////////////////////////////
EventDispatcher* EventCenter::mEventDispatcher = NULL;
//----------------------------------------------------------------------
void EventCenter::init(void)
{
	if (NULL == mEventDispatcher)
	{
		mEventDispatcher = new EventDispatcher();
	}
}
//----------------------------------------------------------------------
void EventCenter::subscribe(int evt, Handler* hd)
{
	init();
	mEventDispatcher->subscribe(evt, hd);
}
//----------------------------------------------------------------------
void EventCenter::subscribe(int evt, CallFunc callfunc)
{
	init();
	mEventDispatcher->subscribe(evt, callfunc);
}
//----------------------------------------------------------------------
void EventCenter::unsubscribe(int evt, Handler* hd)
{
	init();
	mEventDispatcher->unsubscribe(evt, hd);
}
//----------------------------------------------------------------------
void EventCenter::unsubscribe(int evt, CallFunc callfunc)
{
	init();
	mEventDispatcher->unsubscribe(evt, callfunc);
}
//----------------------------------------------------------------------
void EventCenter::unsubscribe(int evt)
{
	init();
	mEventDispatcher->unsubscribe(evt);
}
//----------------------------------------------------------------------
void EventCenter::post(int evt, void* param /* = NULL*/)
{
	init();
	mEventDispatcher->post(evt, param);
}
//----------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// 数据中心
////////////////////////////////////////////////////////////////////////////////////////////////////
DataDispatcher* DataCenter::mDataDispatcher = NULL;
//----------------------------------------------------------------------
void DataCenter::init(void)
{
	if (NULL == mDataDispatcher)
	{
		mDataDispatcher = new DataDispatcher();
	}
}
//----------------------------------------------------------------------
void DataCenter::setInt(int dataId, int data)
{
	init();
	mDataDispatcher->setInt(dataId, data);
}
//----------------------------------------------------------------------
void DataCenter::setUnsignedInt(int dataId, unsigned int data)
{
	init();
	mDataDispatcher->setUnsignedInt(dataId, data);
}
//----------------------------------------------------------------------
void DataCenter::setFloat(int dataId, float data)
{
	init();
	mDataDispatcher->setFloat(dataId, data);
}
//----------------------------------------------------------------------
void DataCenter::setDouble(int dataId, double data)
{
	init();
	mDataDispatcher->setDouble(dataId, data);
}
//----------------------------------------------------------------------
void DataCenter::setChar(int dataId, char data)
{
	init();
	mDataDispatcher->setChar(dataId, data);
}
//----------------------------------------------------------------------
void DataCenter::setString(int dataId, std::string data)
{
	init();
	mDataDispatcher->setString(dataId, data);
}
//----------------------------------------------------------------------
void DataCenter::setBool(int dataId, bool data)
{
	init();
	mDataDispatcher->setBool(dataId, data);
}
//----------------------------------------------------------------------
int DataCenter::getInt(int dataId)
{
	init();
	return mDataDispatcher->getInt(dataId);
}
//----------------------------------------------------------------------
unsigned int DataCenter::getUnsignedInt(int dataId)
{
	init();
	return mDataDispatcher->getUnsignedInt(dataId);
}
//----------------------------------------------------------------------
float DataCenter::getFloat(int dataId)
{
	init();
	return mDataDispatcher->getFloat(dataId);
}
//----------------------------------------------------------------------
double DataCenter::getDouble(int dataId)
{
	init();
	return mDataDispatcher->getDouble(dataId);
}
//----------------------------------------------------------------------
char DataCenter::getChar(int dataId)
{
	init();
	return mDataDispatcher->getChar(dataId);
}
//----------------------------------------------------------------------
std::string DataCenter::getString(int dataId)
{
	init();
	return mDataDispatcher->getString(dataId);
}
//----------------------------------------------------------------------
bool DataCenter::getBool(int dataId)
{
	init();
	return mDataDispatcher->getBool(dataId);
}
//----------------------------------------------------------------------


