/**********************************************************************
 * Author:	jaron.ho
 * Date:    2012-09-06
 * Brief:	common functions
 **********************************************************************/
#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <vector>
#include <functional>

class Common {
public:
    /**********************************************************************
     * type define
     **********************************************************************/
    /* operating system type */
    enum OSType {
        OST_WINDOWS,
        OST_ANDORID,
        OST_IOS
    };

    /**********************************************************************
     * string functions
     **********************************************************************/

    /*
     * Brief:	check if is digit
     * Param:	c - character
     * Return:	bool
     */
    static bool isDigit(char c);

    /*
     * Brief:	check if is digit
     * Param:	str - a string, can be integer or decimals
     * Return:	bool
     */
    static bool isDigit(const std::string& str);

    /*
     * Brief:	string to int
     * Param:	str - a string
     * Return:	int
     */
    static int toInt(const std::string& str);

    /*
     * Brief:	string to long
     * Param:	str - a string
     * Return:	long
     */
    static long toLong(const std::string& str);

    /*
     * Brief:	string to long long
     * Param:	str - a string
     * Return:	long long
     */
    static long long toLongLong(const std::string& str);

    /*
     * Brief:	string to double
     * Param:	str - a string
     * Return:	double
     */
    static double toDouble(const std::string& str);

    /*
     * Brief:	short to string
     * Param:	u - a short number
     * Return:	string
     */
    static std::string toString(short u);

    /*
     * Brief:	int to string
     * Param:	n - a int number
     * Return:	string
     */
    static std::string toString(int n);

    /*
     * Brief:	unsigned int to string
     * Param:	un - a unsigned int number
     * Return:	string
     */
    static std::string toString(unsigned int un);

    /*
     * Brief:	long to string
     * Param:	l - a long number
     * Return:	string
     */
    static std::string toString(long l);

    /*
     * Brief:	unsigned long to string
     * Param:	ul - a unsigned long number
     * Return:	string
     */
    static std::string toString(unsigned long ul);

    /*
     * Brief:	long long to string
     * Param:	ll - a long long number
     * Return:	string
     */
    static std::string toString(long long ll);

    /*
     * Brief:	unsigned long long to string
     * Param:	ull - a unsigned long long number
     * Return:	string
     */
    static std::string toString(unsigned long long ull);

    /*
     * Brief:	float to string
     * Param:	f - a float number
     * Return:	string
     */
    static std::string toString(float f);

    /*
     * Brief:	double to string
     * Param:	d - a double number
     * Return:	string
     */
    static std::string toString(double d);
    
    /*
     * Brief:	convert a decimalism number to binary string
     * Param:	n - a decimalism number
     *          bit - binary complement
     * Return:	string
     */
    static std::string dec2bin(unsigned int n, unsigned int bit = 32);

    /*
     * Brief:	string to lower case
     * Param:	str - a string
     * Return:	string
     */
    static std::string toLower(std::string str);

    /*
     * Brief:	string to upper case
     * Param:	str - a string
     * Return:	string
     */
    static std::string toUpper(std::string str);

    /*
     * Brief:	return a new line string
     * Param:	void
     * Return:	string
     */
    static std::string newLineString(void);

    /*
     * Brief:	return a string which is combined by n ch, e.g. stringOfChar(5, '0') return "00000"
     * Param:	n - count
     *          ch - a character
     * Return:	string
     */
    static std::string stringOfChar(unsigned int n, char ch);

    /*
     * Brief:	limit the string length
     * Param:	str - a string
     *          len - limit length
     *          sufStr - suffix string
     *          defStr - default string when str is empty
     * Return:	string
     */
    static std::string limitStringLength(std::string str, size_t len, const std::string& sufStr = "", const std::string& defStr = "");

    /*
     * Brief:	limit the wstring length
     * Param:	wstr - a wstring
     *          len - limit length
     *          sufWstr - suffix wstring
     *          defWstr - default wstring when wstr is empty
     * Return:	wstring
     */
    static std::wstring limitWstringLength(std::wstring wstr, size_t len, const std::wstring& sufWstr = L"", const std::wstring& defWstr = L"");

    /*
     * Brief:	split a string into vector
     * Param:	str - a string
     *          pattern - split separator
     * Return:	vector<string>
     */
    static std::vector<std::string> splitString(std::string str, const std::string& pattern);

    /*
     * Brief:	replace in a string
     * Param:	str - a string
     *          rep - the specified string that will be replaced
     *          dest - the string that will replace the specified string
     * Return:	string
     */
    static std::string replaceString(std::string str, const std::string& rep, const std::string& dest);

    /*
     * Brief:	format to string
     * Param:	format - string format
     *          ... - variable length argument
     * Return:	string
     */
    static std::string formatString(const char* format, ...);
    
    /*
     * Brief:	wchar_t* to char*
     * Param:	wstr - a wchar_t string
     * Return:	char*
     */
    static char* wchar2char(const wchar_t* wstr);
    
    /*
     * Brief:	char* to wchar_t*
     * Param:	str - a string
     * Return:	wchar_t*
     */
    static wchar_t* char2wchar(const char* str);

    /*
     * Brief:	wstring to string
     * Param:	wstr - a wstring
     *          locale - coding ideftify, e.g. "chs"
     * Return:	string
     */
    static std::string wstring2string(const std::wstring& wstr, const char* locale = "");

    /*
     * Brief:	string to wstring
     * Param:	str - a string
     *          locale - coding ideftify, e.g. "chs"
     * Return:	wstring
     */
    static std::wstring string2wstring(const std::string& str, const char* locale = "");
    
    /*
     * Brief:	check whether string is utf-8
     * Param:	str - a string
     * Return:	bool
     */
    static bool isStringUTF8(const char* str);
    
    /*
     * Brief:	check whether string is gbk
     * Param:	str - a string
     * Return:	bool
     */
    static bool isStringGBK(const char* str);

    /*
     * Brief:	calculate character placeholder
     * Param:	ch - character
     * Return:	place holder
     */
    size_t characterPlaceholder(unsigned char ch);

    /**********************************************************************
     * file functions
     **********************************************************************/

    /*
     * Brief:	create dir
     * Param:	dirName - directory name
     * Return:	bool
     */
    static bool createDir(const std::string& dirName);

    /*
     * Brief:	remove dir
     * Param:	dirName - directory name
     * Return:	void
     */
    static void removeDir(const std::string& dirName);

    /*
     * Brief:	check if file exist
     * Param:	filePath - file path, can be absolute path (d:/test.txt) or relative path (temp/test.txt)
     * Return:	bool
     */
    static bool existFile(const std::string& filePath);

    /*
     * Brief:	create file
     * Param:	filePath - file path, can be absolute path (d:/test.txt) or relative path (temp/test.txt)
     * Return:	bool
     */
    static bool createFile(const std::string& filePath);

    /*
     * Brief:	remove file
     * Param:	filePath - file path, can be absolute path (d:/test.txt) or relative path (temp/test.txt)
     * Return:	bool
     */
    static bool removeFile(const std::string& filePath);

    /*
     * Brief:	rename file
     * Param:	oldFileName - old file name
     *          newFileName - new file name
     *          forceRename - if exist new name file, do force rename
     * Return:	bool
     */
    static bool renameFile(const std::string& oldFileName, const std::string& newFileName, bool forceRename = false);

    /*
     * Brief:	calculate file size
     * Param:	filePath - file path, can be absolute path (d:/test.txt) or relative path (temp/test.txt)
     * Return:	long (-1 not exist file;>0 file size)
     */
    static long calcFileSize(const std::string& filePath);

    /*
     * Brief:	strip file info, e.g. D:\test.txt -> D:\,test.txt,test,.txt
     * Param:	filePath - file path, can be absolute path (d:/test.txt) or relative path (temp/test.txt)
     * Return:	vector<string>
     */
    static std::vector<std::string> stripFileInfo(const std::string& filePath);

    /*
     * Brief:	get file data (remember must delete the value that is allocated in the function)
     * Param:	filePath - file path, can be absolute path (d:/test.txt) or relative path (temp/test.txt)
     *          fileSize - file size, will be calculate in the function
     *          isText - if the file is text type or not
     * Return:	unsigned char*
     */
    static unsigned char* getFileData(const std::string& filePath, long* fileSize, bool isText = true);

    /*
     * Brief:	get file data, data will be seperate line by line into vector
     * Param:	filePath - file path, can be absolute path (d:/test.txt) or relative path (temp/test.txt)
     * Return:	vector<string>
     */
    static std::vector<std::string> getFileDataEx(const std::string& filePath);

    /*
     * Brief:	wirte data into file
     * Param:	data - data that will be written into file
     *          dataSize - data size
     *          filePath - file path, can be absolute path (d:/test.txt) or relative path (temp/test.txt)
     * Return:	bool
     */
    static bool writeDataToFile(const unsigned char* data, long dataSize, const std::string& filePath);

    /*
     * Brief:	copy src file to dest file
     * Param:	srcFilePath - src file, can be absolute path (d:/test.txt) or relative path (temp/test.txt)
     *          destFilePath - dest file, can be absolute path (d:/test.txt) or relative path (temp/test.txt)
     * Return:	bool
     */
    static bool copyFile(const std::string& srcFilePath, const std::string& destFilePath);

    /*
     * Brief:	revisal path, replace "\\" to "/", make sure end of path is not "/" and "\\"
     * Param:	path - can be absolute path (d:/) or relative path (temp/)
     * Return:	string
     */
    static std::string revisalPath(std::string path);

    /*
     * Brief:	check if path is absolute or not
     * Param:	path - path
     *          os - OST_WINDOWS, OST_ANDORID, OST_IOS
     * Return:	bool
     */
    static bool isAbsolutePath(std::string path, OSType os = OST_WINDOWS);

    /*
     * Brief:	get full path (absolute path)
     * Param:	path - path
     *          os - OST_WINDOWS, OST_ANDORID, OST_IOS
     * Return:	string
     */
    static std::string getFullPath(std::string path, OSType os = OST_WINDOWS);

    /*
     * Brief:	get current path (absolute path)
     * Param:	void
     * Return:	string
     */
    static std::string getCurrentDir(void);

    /*
     * Brief:	get parent path
     * Param:	dir - directory
     * Return:	string
     */
    static std::string getParentDir(std::string dir = "");

    /*
     * Brief:	recursion search files in the specified directory
     * Param:	dirName - specified directory, can be absolute path (d:) or relative path (temp)
     *          extList - file extension
     *          callback - called when file searched
     *          recursive - if recursive sub dir
     * Return:	void
     */
    static void searchFile(std::string dirName, const std::vector<std::string>& extList, 
                           std::function<void(const std::string& fileName, 
                                              unsigned long fileSize, 
                                              long createTime, 
                                              long writeTime, 
                                              long accessTime)> callback, 
                           bool recursive = true);

    /*
     * Brief:	recursion serach directory in the specified directory
     * Param:	dirName - specified directory, can be absolute path (d:) or relative path (temp)
     *          callback - called when directory searched
     *          recursive - if recursive sub dir
     * Return:	void
     */
    static void searchDir(std::string dirName, 
                          std::function<void(const std::string& dirName, 
                                             long createTime, 
                                             long writeTime, 
                                             long accessTime)> callback, 
                          bool recursive = true);

    /**********************************************************************
    * network functions
    **********************************************************************/

    /*
     * Brief:	check whether string is ip format
     * Param:	str - ip string, e.g. "157.15.3.71"
     * Return:	bool
     */
    static bool isIpFormat(std::string ip);

    /*
     * Brief:	check whether ip is inner
     * Param:	str - ip string, e.g. "157.15.3.71"
     *          ret - check result
     * Return:	int - 0.check ok, 1.ip format is error
     */
    static int isInnerIp(std::string ip, bool* ret);

    /**********************************************************************
     * time functions
     **********************************************************************/

    /*
     * Brief:	get current system time (current from 1970-01-01 00:00:00)
     * Param:	void
     * Return:	double (seconds)
     */
    static double getTime(void);

    /*
     * Brief:	get current date
     * Param:	year - current year [1900, )
     *          mon - current month [1, 12]
     *          mday - current day in month [1, 31]
     *          hour - current hour [0, 23]
     *          min - current minute [0, 59]
     *          sec - current seconds [0, 60]
     *          wday - current day in week [1, 7]: Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday
     *          yday - current day in year [1, 366]
     * Return:	void
     */
    static void getDate(int* year, int* mon, int* mday, int* hour = NULL, int* min = NULL, int* sec = NULL, int* wday = NULL, int* yday = NULL);

    /*
     * Brief:	time to date
     * Param:	seconds - seconds, if = 0, get current date
     * Return:	tm
     */
    static struct tm timeToDate(long seconds = 0);

    /*
     * Brief:	date to time
     * Param:	y - year, must > 1970
     *          m - month , [1, 12]
     *          d - day, [1, 31]
     *          h - hour, [0, 23]
     *          n - minute, [0, 59]
     *          s - second, [0, 59]
     * Return:	long (seconds)
     */
    static long dateToTime(int y = 1970, int m = 1, int d = 1, int h = 8, int n = 0, int s = 0);

    /**********************************************************************
     * other functions
     **********************************************************************/
     
    /*
     * Brief:   generate uid
     * Param:   void
     * Return:  unsigned long long
     */
    static unsigned long long generateUID(void);
    
    /*
     * Brief:   生成文件名
     * Param:   extname - 文件后缀名
     * Return:  std::string
     */
    static std::string generateFilename(const std::string& extname);

    /*
     * Brief:	get size of array, e.g.
     *              int arr[] = {0, 1, 2, 3};
     *              size_t len = Common::getArrayCount<int>(arr);
     * Param:	arr - array
     * Return:	size_t
     */
    template <typename T, typename TArray>
    static size_t getArrayCount(const TArray& arr) {
        return sizeof(arr) / sizeof(T);
    }

    /*
     * Brief:	get data from a specify array, e.g.
     *              std::vector<int> vec;
     *              size_t pg = Common::getArray<int>(vec, 1, 3);
     * Param:	arr - array
     *          beginIndex - array index, start from 0
     *          num - number
     * Return:	size_t
     */
    template <typename T>
    static std::vector<T> getArray(const std::vector<T>& arr, size_t beginIndex, size_t num) {
        std::vector<T> vec;
        size_t len = arr.size();
        if (0 == len || beginIndex < 0 || beginIndex >= len) {
            return vec;
        }
        size_t endIndex = beginIndex + num;
        if (endIndex >= len) {
            endIndex = len;
        }
        for (size_t i = beginIndex; i < endIndex; ++i) {
            vec.push_back(arr[i]);
        }
        return vec;
    }

    /*
     * Brief:	get count of pages, [1,2,3],[4,5,6],[7,8,9] = 3, e.g.
     *              std::vector<int> vec;
     *              size_t pg = Common::getPageCount<int>(vec, 3);
     * Param:	arr - array
     *          numPerPage - number per page, start from 1
     * Return:	size_t
     */
    template <typename T>
    static size_t getPageCount(const std::vector<T>& arr, size_t numPerPage) {
        if (0 == numPerPage) {
            return 0;
        }
        size_t len = arr.size();
        if (0 == len) {
            return 0;
        }
        size_t div = len / numPerPage;
        size_t rem = len % numPerPage;
        return 0 == rem ? div : div + 1;
    }

    /*
     * Brief:	get data by specify page, [1,2,3],[4,5,6],[7,8,9], second page = [4,5,6], e.g.
     *              std::vector<int> vec;
     *              std::vector<int> temp = Common::getPage<int>(vec, 3, 2);
     * Param:	arr - array
     *          numPerPage - number per page
     *          curPage - specify page, start from 0
     * Return:	size_t
     */
    template <typename T>
    static std::vector<T> getPage(const std::vector<T>& arr, size_t numPerPage, size_t curPage) {
        std::vector<T> vec;
        size_t pageCount = getPageCount<T>(arr, numPerPage);
        if (0 == pageCount || curPage < 0 || curPage >= pageCount) {
            return vec;
        }
        size_t len = arr.size();
        size_t beginIndex = numPerPage * curPage;
        if (len < numPerPage) {
            beginIndex = 0;
        }
        size_t endIndex = beginIndex + numPerPage;
        if (endIndex > len) {
            endIndex = len;
        }
        for (size_t i = beginIndex; i < endIndex; ++i) {
            vec.push_back(arr[i]);
        }
        return vec;
    }
};

#endif	/* _COMMON_H_ */
