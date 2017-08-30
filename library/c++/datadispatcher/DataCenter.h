/**********************************************************************
* Author:	jaron.ho
* Date:		2013-09-16
* Brief:	data center
**********************************************************************/
#ifndef _DATA_CENTER_H_
#define _DATA_CENTER_H_

#include "DataDispatcher.h"

/*
 * Brief:	数据中心
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

#endif	// _DATA_CENTER_H_
