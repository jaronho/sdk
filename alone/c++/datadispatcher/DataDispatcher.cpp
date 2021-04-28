/**********************************************************************
* Author:	jaron.ho
* Date:		2012-09-06
* Brief:	data dispatcher
**********************************************************************/
#include "DataDispatcher.h"

DataDispatcher::DataDispatcher(void) {
}

DataDispatcher::~DataDispatcher(void) {
	mIntMap.clear();
	mUnsignedIntMap.clear();
	mFloatMap.clear();
	mDoubleMap.clear();
	mCharMap.clear();
	mStringMap.clear();
	mBoolMap.clear();
}

void DataDispatcher::setInt(int dataId, int data) {
	if (mIntMap.end() == mIntMap.find(dataId)) {
		mIntMap.insert(std::make_pair(dataId, data));
	}
	mIntMap[dataId] = data;
}

void DataDispatcher::setUnsignedInt(int dataId, unsigned int data) {
	if (mUnsignedIntMap.end() == mUnsignedIntMap.find(dataId)) {
		mUnsignedIntMap.insert(std::make_pair(dataId, data));
	}
	mUnsignedIntMap[dataId] = data;
}

void DataDispatcher::setFloat(int dataId, float data) {
	if (mFloatMap.end() == mFloatMap.find(dataId)) {
		mFloatMap.insert(std::make_pair(dataId, data));
	}
	mFloatMap[dataId] = data;
}

void DataDispatcher::setDouble(int dataId, double data) {
	if (mDoubleMap.end() == mDoubleMap.find(dataId)) {
		mDoubleMap.insert(std::make_pair(dataId, data));
	}
	mDoubleMap[dataId] = data;
}

void DataDispatcher::setChar(int dataId, char data) {
	if (mCharMap.end() == mCharMap.find(dataId)) {
		mCharMap.insert(std::make_pair(dataId, data));
	}
	mCharMap[dataId] = data;
}

void DataDispatcher::setString(int dataId, std::string data) {
	if (mStringMap.end() == mStringMap.find(dataId)) {
		mStringMap.insert(std::make_pair(dataId, data));
	}
	mStringMap[dataId] = data;
}

void DataDispatcher::setBool(int dataId, bool data) {
	if (mBoolMap.end() == mBoolMap.find(dataId)) {
		mBoolMap.insert(std::make_pair(dataId, data));
	}
	mBoolMap[dataId] = data;
}

int DataDispatcher::getInt(int dataId) {
	if (mIntMap.end() == mIntMap.find(dataId)) {
		return 0;
	}
	return mIntMap[dataId];
}

unsigned int DataDispatcher::getUnsignedInt(int dataId) {
	if (mUnsignedIntMap.end() == mUnsignedIntMap.find(dataId)) {
		return 0;
	}
	return mUnsignedIntMap[dataId];
}

float DataDispatcher::getFloat(int dataId) {
	if (mFloatMap.end() == mFloatMap.find(dataId)) {
		return 0.0f;
	}
	return mFloatMap[dataId];
}

double DataDispatcher::getDouble(int dataId) {
	if (mDoubleMap.end() == mDoubleMap.find(dataId)) {
		return 0.0f;
	}
	return mDoubleMap[dataId];
}

char DataDispatcher::getChar(int dataId) {
	if (mCharMap.end() == mCharMap.find(dataId)) {
		return '\0';
	}
	return mCharMap[dataId];
}

std::string DataDispatcher::getString(int dataId) {
	if (mStringMap.end() == mStringMap.find(dataId)) {
		return "";
	}
	return mStringMap[dataId];
}

bool DataDispatcher::getBool(int dataId) {
	if (mBoolMap.end() == mBoolMap.find(dataId)) {
		return false;
	}
	return mBoolMap[dataId];
}
