/**********************************************************************
* Author:	jaron.ho
* Date:		2013-09-16
* Brief:	data center
**********************************************************************/
#include "DataCenter.h"

DataDispatcher* DataCenter::mDataDispatcher = nullptr;

void DataCenter::init(void) {
	if (nullptr == mDataDispatcher) {
		mDataDispatcher = new DataDispatcher();
	}
}

void DataCenter::setInt(int dataId, int data) {
	init();
	mDataDispatcher->setInt(dataId, data);
}

void DataCenter::setUnsignedInt(int dataId, unsigned int data) {
	init();
	mDataDispatcher->setUnsignedInt(dataId, data);
}

void DataCenter::setFloat(int dataId, float data) {
	init();
	mDataDispatcher->setFloat(dataId, data);
}

void DataCenter::setDouble(int dataId, double data) {
	init();
	mDataDispatcher->setDouble(dataId, data);
}

void DataCenter::setChar(int dataId, char data) {
	init();
	mDataDispatcher->setChar(dataId, data);
}

void DataCenter::setString(int dataId, std::string data) {
	init();
	mDataDispatcher->setString(dataId, data);
}

void DataCenter::setBool(int dataId, bool data) {
	init();
	mDataDispatcher->setBool(dataId, data);
}

int DataCenter::getInt(int dataId) {
	init();
	return mDataDispatcher->getInt(dataId);
}

unsigned int DataCenter::getUnsignedInt(int dataId) {
	init();
	return mDataDispatcher->getUnsignedInt(dataId);
}

float DataCenter::getFloat(int dataId) {
	init();
	return mDataDispatcher->getFloat(dataId);
}

double DataCenter::getDouble(int dataId) {
	init();
	return mDataDispatcher->getDouble(dataId);
}

char DataCenter::getChar(int dataId) {
	init();
	return mDataDispatcher->getChar(dataId);
}

std::string DataCenter::getString(int dataId) {
	init();
	return mDataDispatcher->getString(dataId);
}

bool DataCenter::getBool(int dataId) {
	init();
	return mDataDispatcher->getBool(dataId);
}
