/**********************************************************************
* 作者：hezhr
* 时间：2012-9-10
* 描述：异常
**********************************************************************/
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <string>
#include <exception>


class Exception : public std::exception
{
public:
	Exception(const std::string &description, const std::string &func, const char *file, long line);
	
	Exception(const Exception &rhs);
	
	void operator = (const Exception &rhs);
	
	~Exception() throw();

public:
	/*
	功	能：返回异常描述
	参	数：void
	返回值：const std::string
	*/
	virtual const std::string& getDescription(void) const;

	/*
	功	能：返回异常所在函数名
	参	数：void
	返回值：const std::string
	*/
	virtual const std::string& getFunction() const;

	/*
	功	能：返回异常所在文件名
	参	数：void
	返回值：const std::string
	*/
	virtual const std::string& getFile() const;

	/*
	功	能：返回异常所在行号
	参	数：void
	返回值：long
	*/
	virtual long getLine() const;

	/*
	功	能：返回异常完整描述，包括函数名，文件名，行号
	参	数：void
	返回值：const std::string
	*/
	virtual const std::string& getFullDescription(void) const;

	/*
	功	能：异常描述，重载std::exception::what()，这里返回完整描述
	参	数：void
	返回值：const char*
	*/
	const char* what() const throw();
	
protected:
	std::string mDescription;						// 异常描述

	std::string mFunction;							// 异常所在函数名

	std::string mFile;								// 异常所在文件名

	long mLine;										// 异常所在行号

	mutable std::string mFullDescription;			// 完整的描述
};


#ifndef EXCEPTION
	#define EXCEPTION(description, func) throw Exception(description, func, __FILE__, __LINE__)
#endif


#endif	// _EXCEPTION_H_


