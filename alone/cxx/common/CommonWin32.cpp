/******************************************************************************
* 作者: hezhr
* 时间: 2012-09-06
* 描述: 用于一些Win32系统通用函数的包装
******************************************************************************/
#include "CommonWin32.h"
#include <ShellAPI.h>
#include <shlobj.h>


//--------------------------------------------------------------------------
void CommonWin32::execDosCommand(const std::string& cmd)
{
	std::string newcmd = "cmd /c " + cmd;
	::WinExec(newcmd.c_str(),SW_HIDE);
}
//--------------------------------------------------------------------------
void CommonWin32::waitExceDosCommand(const std::string& cmd)
{
	std::string newcmd = "cmd.exe /c " + cmd;
	runExe(newcmd, true);
}
//--------------------------------------------------------------------------
void CommonWin32::runExe(const std::string& cmd, bool bBlocking)
{
//	std::string newcmd = cmd;
//
//	PROCESS_INFORMATION           piProcInfo;
//	STARTUPINFO                   siStartInfo;
//
//	ZeroMemory(&piProcInfo, sizeof(piProcInfo));
//	ZeroMemory(&siStartInfo, sizeof(siStartInfo));
//
//	siStartInfo.cb = sizeof(STARTUPINFO);  
//	siStartInfo.lpReserved = NULL;  
//	siStartInfo.lpReserved2 = NULL;  
//	siStartInfo.cbReserved2 = 0;  
//	siStartInfo.lpDesktop = NULL;  
//	siStartInfo.dwFlags	= 0;  
//
//	LPSTR lp_cmd;
//#ifdef UNICODE
//	lp_cmd = UnicodeToANSI(cmd).c_str();
//#else
//	lp_cmd = (LPSTR)cmd.c_str();
//#endif
//	::CreateProcess(NULL, lp_cmd, NULL, NULL, 0, 0, NULL, NULL, &siStartInfo, &piProcInfo);
//
//	if (bBlocking)
//	{
//		::WaitForSingleObject(piProcInfo.hProcess, INFINITE);   
//	}
//	else
//	{
//		::CloseHandle(piProcInfo.hProcess);
//	}
}
//--------------------------------------------------------------------------
std::wstring CommonWin32::ANSIToUnicode(const char* str)
{
	int textlen = ::MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0); 	
	std::wstring buf(textlen, 0);
	::MultiByteToWideChar(CP_ACP, 0, str, -1, const_cast<LPWSTR>(buf.c_str()) , buf.size()); 
	return buf.c_str(); 
}
//--------------------------------------------------------------------------
std::string CommonWin32::UnicodeToANSI(const wchar_t* str)
{
	int textlen = ::WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	std::string buf(textlen, 0);
	::WideCharToMultiByte(CP_ACP, 0, str, -1, const_cast<LPSTR>(buf.c_str()), buf.size(), NULL, NULL);
	return buf.c_str();
}
//--------------------------------------------------------------------------
std::wstring CommonWin32::GB2312ToUnicode(const char* str)
{
	int textlen = ::MultiByteToWideChar(936, 0, str, -1, NULL, 0); 	
	std::wstring buf(textlen, 0);
	::MultiByteToWideChar(936, 0, str, -1, const_cast<LPWSTR>(buf.c_str()) , buf.size()); 
	return buf.c_str(); 
}
//--------------------------------------------------------------------------
std::string CommonWin32::UnicodeToGB2312(const wchar_t* str)
{
	int textlen = ::WideCharToMultiByte(936, 0, str, -1, NULL, 0, NULL, NULL);
	std::string buf(textlen, 0);
	::WideCharToMultiByte(936, 0, str, -1, const_cast<LPSTR>(buf.c_str()), buf.size(), NULL, NULL);
	return buf.c_str();
}
//--------------------------------------------------------------------------
std::wstring CommonWin32::UTF8ToUnicode(const char* str)
{
	int textlen = ::MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0); 	
	std::wstring buf(textlen, 0);
	::MultiByteToWideChar(CP_UTF8, 0, str, -1, const_cast<LPWSTR>(buf.c_str()), buf.size()); 
	return buf.c_str(); 
}
//--------------------------------------------------------------------------
std::string CommonWin32::UnicodeToUTF8(const wchar_t* str)
{
	int textlen = ::WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	std::string buf(textlen, 0);
	::WideCharToMultiByte(CP_UTF8, 0, str, -1, const_cast<LPSTR>(buf.c_str()), buf.size(), NULL, NULL);
	return buf.c_str();
}
//--------------------------------------------------------------------------
std::string CommonWin32::GB2312ToUTF8(const char* str)
{
	return UnicodeToUTF8(GB2312ToUnicode(str).c_str());
}
//--------------------------------------------------------------------------
std::string CommonWin32::UTF8ToGB2312(const char* str)
{
	return UnicodeToGB2312(UTF8ToUnicode(str).c_str());
}
//--------------------------------------------------------------------------
int CommonWin32::UTF8Length(const char* str)
{
	return (::MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0) - 1);		// 减1是末尾\0结束符的字节 	
}
//--------------------------------------------------------------------------
std::string CommonWin32::getDateTimeString(const ::SYSTEMTIME& st)
{
	const int maxLength = 32;
	char buf[maxLength];
	sprintf_s(buf, "%04d%02d%02d%02d%02d%02d%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return buf;
}
//--------------------------------------------------------------------------
std::string CommonWin32::getCurDateTimeString(void)
{
	::SYSTEMTIME st;
	::GetLocalTime(&st);
	return getDateTimeString(st);
}
//--------------------------------------------------------------------------
std::string CommonWin32::formatDateTime(std::string format, const ::SYSTEMTIME& st)
{
	return CommonFunc::formatDateTime(format, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}
//--------------------------------------------------------------------------
bool CommonWin32::existDirectory(const std::string& path)
{
	::WIN32_FIND_DATAA wfd;
    HANDLE hFind = ::FindFirstFileA(path.c_str(), &wfd);
    ::FindClose(hFind);
	return ((hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}
//--------------------------------------------------------------------------
bool CommonWin32::existDir(const std::string& path)
{
	if (path.empty())
		return false;

	char ch = path[path.size() - 1];
	if ('/' == ch || '\\' == ch) 
		return existDirectory(path.substr(0, path.size() - 1));

	return existDirectory(path);
}
//--------------------------------------------------------------------------
bool CommonWin32::forceDirectory(const std::string& dir)
{
	return 0 != ::CreateDirectoryA(dir.c_str(), NULL);
}
//--------------------------------------------------------------------------
bool CommonWin32::forceDir(const std::string& dir)
{
	if (dir.empty())
		return false;

	for (size_t start = 0; start < dir.size();)
	{
		size_t pos = dir.find_first_of("/\\", start);
		if (pos < dir.size())
		{
			std::string temp = dir.substr(0, pos);
			if (false == existDir(temp))
			{
				if (false == forceDirectory(temp))
					return false;
			}
			start = pos + 1;
		}
		else
		{
			break;
		}
	}

	if (false == existDir(dir))
		return forceDirectory(dir);
	
	return true;
}
//--------------------------------------------------------------------------
std::string CommonWin32::getCurrDir(void)
{
	const int length = 1024;
	char buf[length];
	::GetCurrentDirectoryA(length, buf);
	std::string str = buf;
	str.push_back('\\');
	return str;
}
//--------------------------------------------------------------------------
std::string CommonWin32::getDirName(const std::string& dir)
{
	size_t pos = dir.find_last_of("/\\:");
	if (pos < dir.size())
		return dir.substr(0, pos + 1);
	
	return "";
}
//--------------------------------------------------------------------------
void CommonWin32::openDir(const std::string& dir)
{
//	std::string open = "open";
//	LPSTR lp_open;
//	LPSTR lp_dir;
//#ifdef UNICODE
//	lp_open = UnicodeToANSI(cmd).c_str();
//	lp_dir = UnicodeToANSI(dir).c_str();
//#else
//	lp_open = (LPSTR)open.c_str();
//	lp_dir = (LPSTR)dir.c_str();
//#endif
//	::ShellExecute(NULL, lp_open, lp_dir, NULL, NULL, SW_SHOWNORMAL);
}
//--------------------------------------------------------------------------
std::string CommonWin32::getFileName(const std::string& file)
{
	size_t pos = file.find_last_of("/\\:");
	if (pos < file.size())
		return file.c_str() + pos + 1;
	
	return file;
}
//--------------------------------------------------------------------------
void CommonWin32::getFileNameAndExt(const std::string& file, std::string& name, std::string& ext)
{
	ext = "";
	name = getFileName(file);
	size_t pos = name.find_last_of('.');
	if (pos < name.size())
	{
		ext = name.c_str() + pos;
		name.resize(pos);
	}
}
//--------------------------------------------------------------------------
void CommonWin32::getDirAndFileName(const std::string& file, std::string& dir, std::string& name)
{
	size_t pos = file.find_last_of("/\\:");
	if (pos < file.size())
	{
		dir = file.substr(0, pos + 1);
		name = file.c_str() + pos + 1;
	}
	else
	{
		dir = "";
		name = file;
	}	
}
//--------------------------------------------------------------------------
std::string CommonWin32::getFullPath(const std::string& file)
{	
	char buf[1024];
	::GetFullPathNameA(file.c_str(), 1024, buf, NULL);
	return buf;
}
//--------------------------------------------------------------------------
std::string CommonWin32::getModuleName(void)
{
	const int length = 1024;
	char szModuleName[length];
	::GetModuleFileNameA(NULL, szModuleName, length);

	std::string module_name, ext;		
	getFileNameAndExt(szModuleName, module_name, ext);
	return module_name;
}
//--------------------------------------------------------------------------
std::wstring CommonWin32::getSelectPathDir(void)
{
	std::wstring pathDir;
	LPMALLOC pMalloc;
	if (NOERROR == ::SHGetMalloc(&pMalloc))
	{
		TCHAR pszBuffer[MAX_PATH];
		for (int i=0; i<MAX_PATH; ++i)
		{
			pszBuffer[i] = ' ';		// 将pszBuffer全部赋为空格
		}

		BROWSEINFO bi;
		ZeroMemory(&bi, sizeof(BROWSEINFO));
		bi.pidlRoot			= NULL;
		bi.pszDisplayName	= pszBuffer;
		bi.ulFlags			= BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
		bi.lpfn				= NULL;

		LPITEMIDLIST pidl;
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)
		{
			::SHGetPathFromIDList(pidl, pszBuffer);
			pathDir = pszBuffer;
			pMalloc->Free(pidl);
		}
		pMalloc->Release();
	}
	return pathDir;
}
//--------------------------------------------------------------------------
void CommonWin32::browseAllFiles(std::wstring dir, std::wstring ext, std::vector<std::wstring>& vec)
{
	if (0 == dir.compare(L"") || 0 == dir.size())
		return;

	size_t dirSize = dir.size();
	if (dirSize >=2 && 0 != dir.substr(dirSize, 2).compare(L"//"))
	{
		dir += L"//";
	}
	dir = dir + L"*.*";
	
	std::wstring strPath;
	CFileFind finder;
	BOOL bWorking = finder.FindFile(dir.c_str());
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		strPath = finder.GetFilePath();
		if (finder.IsDots())
			continue;

		if (finder.IsDirectory())
		{
			browseAllFiles(strPath, ext, vec); // 递归调用
		}
		else
		{
			// strPath就是所要获取的文件路径
			size_t extSize = ext.size();
			size_t pathSize = strPath.size();
			if (0 == ext.compare(L"") || (pathSize >= extSize && 0 == strPath.substr(pathSize - extSize, extSize).compare(ext)))
			{
				vec.push_back(strPath);
			}
		}
	}
}
//--------------------------------------------------------------------------