/******************************************************************************
* 作者: hezhr
* 时间: 2012-09-06
* 描述: 用于一些Win32系统通用函数的包装
******************************************************************************/
#ifndef _COMMON_WIN32_H_
#define _COMMON_WIN32_H_

#include "Common.h"
#include <afx.h>

class CommonWin32 : public Common
{
public:
	//----------------------------------------------------------------------
	// Win32 API 包装
	//----------------------------------------------------------------------

	/*
	功	能：执行Dos的命令，注意这开了一个新进程
	参	数：cmd - 比如copy指令.
	返回值：void
	*/
	static void execDosCommand(const std::string& cmd);

	/*
	功	能：等待执行Dos的命令，此函数慎用，这个函数会等Dos命令的执行进程结束后才继续，注意这是一个进程的等待
	参	数：cmd - 指令
	返回值：void
	*/
	static void waitExceDosCommand(const std::string& cmd);

	/*
	功	能：执行exe
	参	数：cmd - 指令；bBlocking - 为true时会等待exe执行完毕才返回
	返回值：void
	*/
	static void runExe(const std::string& cmd, bool bBlocking);

	//----------------------------------------------------------------------
	// 字符串相关函数
	//----------------------------------------------------------------------

	/*
	功	能：ANSI转成Unicode
	参	数：str - 字符串
	返回值：wstring
	*/
	static std::wstring ANSIToUnicode(const char* str);

	/*
	功	能：Unicode转成ANSI
	参	数：str - 字符串
	返回值：wstring
	*/
	static std::string UnicodeToANSI(const wchar_t* str);

	/*
	功	能：GBK转成Unicode
	参	数：str - 字符串
	返回值：wstring
	*/
	static std::wstring GB2312ToUnicode(const char* str);

	/*
	功	能：Unicode转成GBK
	参	数：str - 字符串
	返回值：string
	*/
	static std::string UnicodeToGB2312(const wchar_t* str);

	/*
	功	能：UTF8转成Unicode
	参	数：str - 字符串
	返回值：wstring
	*/
	static std::wstring UTF8ToUnicode(const char* str);

	/*
	功	能：Unicode转成UTF8
	参	数：str - 字符串
	返回值：string
	*/
	static std::string UnicodeToUTF8(const wchar_t* str);

	/*
	功	能：GBK转成UTF8
	参	数：str - 字符串
	返回值：string
	*/
	static std::string GB2312ToUTF8(const char* str);

	/*
	功	能：UTF8转成GBK
	参	数：str - 字符串
	返回值：string
	*/
	static std::string UTF8ToGB2312(const char* str);

	/*
	功	能：UTF8长度
	参	数：str - 字符串
	返回值：int
	*/
	static int UTF8Length(const char* str);

	//----------------------------------------------------------------------
	// 时间相关函数		
	//----------------------------------------------------------------------

	/*
	功	能：取得日期时间字串(yyyymmddhhnnssxxx) 
	参	数：st - 结构体
	返回值：string
	*/
	static std::string getDateTimeString(const ::_SYSTEMTIME& st);
 
	/*
	功	能：取得当前日期时间字串(yyyymmddhhnnssxxx)
	参	数：void
	返回值：string
	*/
	static std::string getCurDateTimeString(void);

	/*
	功	能：格式化日期时间
	参	数：format - 时间格式字串；st - 日期结果
	返回值：string
	*/
	static std::string formatDateTime(std::string sFormat, const ::SYSTEMTIME& st);

	//----------------------------------------------------------------------
	// 文件目录相关函数
	//----------------------------------------------------------------------

	/*
	功	能：判断目录是否存在，一般调用existDir
	参	数：path - 路径（完整路径）
	返回值：bool
	*/
	static bool existDirectory(const std::string& path);

	/*
	功	能：判断目录是否存在
	参	数：path - 路径（完整路径）
	返回值：bool
	*/
	static bool existDir(const std::string& path);

	/*
	功	能：建立目录，一般调用forceDir
	参	数：dir - 目录
	返回值：bool
	*/
	static bool forceDirectory(const std::string& dir);

	/*
	功	能：建立目录
	参	数：dir - 目录
	返回值：bool
	*/
	static bool forceDir(const std::string& dir);

	/*
	功	能：取得当前目录,返回目录名会自动加上'\'号
	参	数：void
	返回值：string
	*/
	static std::string getCurrDir(void);

	/*
	功	能：取得目录名
	参	数：dir - 完整目录
	返回值：string
	*/
	static std::string getDirName(const std::string& dir);

	/*
	功	能：打开目录
	参	数：dir - 目录
	返回值：void
	*/
	static void openDir(const std::string& dir);

	/*
	功	能：提取文件名，文件名包含扩展名如(myfile.bmp)
	参	数：file - 完整文件名（包括路径）
	返回值：string
	*/
	static std::string getFileName(const std::string& file);

	/*
	功	能：提取文件名及扩展名
	参	数：file - 完整文件名（包括路径）；name - 文件名不包含扩展名；ext - 包含点号如".exe"
	返回值：void
	*/
	static void getFileNameAndExt(const std::string& file, std::string& name, std::string& ext);

	/*
	功	能：提取目录名，文件名，文件名包含扩展名如(myfile.bmp),返回目录名会自动加上'\'号
	参	数：file - 完整文件名（包括路径）；dir - 路径；name - 文件名不包含扩展名；
	返回值：void
	*/
	static void getDirAndFileName(const std::string& file, std::string& dir, std::string& name);

	/*
	功	能：提取完整路径，主要是替换掉"..\", ".\"之类的符号
	参	数：file - 文件名
	返回值：string
	*/
	static std::string getFullPath(const std::string& file);

	/*
	功	能：取得程序名称，例：假设当前应用程序名称为D:\\myPath\\myApp.exe, 那结果为"myApp"
	参	数：void
	返回值：string
	*/
	static std::string getModuleName(void);

	/*
	功	能：打开一个路径选择框，获取选择的文件目录路径
	参	数：void
	返回值：wstring
	*/
	static std::wstring getSelectPathDir(void);

	/*
	功	能：遍历指定目录下的所有文件
	参	数：dir - 指定目录；ext - 文件后缀名（.lua）；vec - 获取到的的文件
	返回值：void
	*/
	static void browseAllFiles(std::wstring dir, std::wstring ext, std::vector<std::wstring>& vec);
};

#endif	// _COMMON_WIN32_H_