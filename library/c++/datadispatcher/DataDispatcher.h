/**********************************************************************
* Author:	jaron.ho
* Date:		2012-09-06
* Brief:	data dispatcher
**********************************************************************/
#ifndef _DATA_DISPATCHER_H_
#define _DATA_DISPATCHER_H_

#include <string>
#include <map>

/*
 * Brief:	数据派发器
 */
class DataDispatcher {
public:
	DataDispatcher(void);

	~DataDispatcher(void);
	
public:
	/*
	 * Brief:	设置整型数据
	 * Param:	dataId - 数据标识
	 *			data - 整型数据
	 * Return:	void
	 */
	void setInt(int dataId, int data);

	/*
	 * Brief:	设置无符号整型数据
	 * Param:	dataId - 数据标识
	 *			data - 无符号整型数据
	 * Return:	void
	 */
	void setUnsignedInt(int dataId, unsigned int data);

	/*
	 * Brief:	设置单精度浮点型数据
	 * Param:	dataId - 数据标识
	 *			data - 单精度浮点型数据
	 * Return:	void
	 */
	void setFloat(int dataId, float data);

	/*
	 * Brief:	设置双精度浮点型数据
	 * Param:	dataId - 数据标识
	 *			data - 双精度浮点型数据
	 * Return:	void
	 */
	void setDouble(int dataId, double data);

	/*
	 * Brief:	设置单字符数据
	 * Param:	dataId - 数据标识
	 *			data - 单字符数据
	 * Return:	void
	 */
	void setChar(int dataId, char data);

	/*
	 * Brief:	设置字符串数据
	 * Param:	dataId - 数据标识
	 *			data - 字符串数据
	 * Return:	void
	 */
	void setString(int dataId, std::string data);

	/*
	 * Brief:	设置布尔型数据
	 * Param:	dataId - 数据标识
	 *			data - 布尔型数据
	 * Return:	void
	 */
	void setBool(int dataId, bool data);

	/*
	 * Brief:	获取整型数据
	 * Param:	dataId - 数据标识
	 * Return:	int
	 */
	int getInt(int dataId);

	/*
	 * Brief:	获取无符号整型数据
	 * Param:	dataId - 数据标识
	 * Return:	unsigned int
	 */
	unsigned int getUnsignedInt(int dataId);

	/*
	 * Brief:	获取单精度浮点型数据
	 * Param:	dataId - 数据标识
	 * Return:	float
	 */
	float getFloat(int dataId);

	/*
	 * Brief:	获取双精度浮点型数据
	 * Param:	dataId - 数据标识
	 * Return:	double
	 */
	double getDouble(int dataId);

	/*
	 * Brief:	获取单字符数据
	 * Param:	dataId - 数据标识
	 * Return:	char
	 */
	char getChar(int dataId);

	/*
	 * Brief:	获取字符串数据
	 * Param:	dataId - 数据标识
	 * Return:	std::string
	 */
	std::string getString(int dataId);

	/*
	 * Brief:	获取布尔型数据
	 * Param:	dataId - 数据标识
	 * Return:	bool
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

#endif	// _DATA_DISPATCHER_H_
