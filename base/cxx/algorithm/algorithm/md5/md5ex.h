#pragma once
#include <functional>
#include <string>
#include <vector>

namespace algorithm
{
/** 
 * @brief md5加密字符串
 * @param input 原始字节流
 * @param inputLen 输入的字节流长度
 * @return md5字符串(32位小写)
 */
std::string md5SignStrEx(const unsigned char* input, size_t inputLen);

/** 
 * @brief md5加密字符串列表
 * @param strList 字符串列表
 * @param sortFlag 是否对列表进行排序: 0-不排序, 1-从小到大排序, 2-从大到小排序
 * @return md5字符串(32位小写)
 */
std::string md5SignStrList(std::vector<std::string> strList, int sortFlag = 0);

/** 
 * @brief md5加密文件
 * @param handle 文件句柄
 * @param blockSize 每次读取的文件块大小(字节), 最小1024字节
 * @param stopFunc 停止函数, 返回值: true-停止, false-继续
 * @return md5字符串(32位小写)
 */
std::string md5SignFileHandleEx(FILE* handle, size_t blockSize = 1024, const std::function<bool()>& stopFunc = nullptr);

/** 
 * @brief md5加密文件
 * @param filename 文件路径
 * @param blockSize 每次读取的文件块大小(字节), 最小1024字节
 * @param stopFunc 停止函数, 返回值: true-停止, false-继续
 * @return md5字符串(32位小写)
 */
std::string md5SignFileEx(const std::string& filename, size_t blockSize = 1024, const std::function<bool()>& stopFunc = nullptr);
} // namespace algorithm