#pragma once
#include <functional>
#include <string>
#include <vector>

#include "xxhash.h"

namespace algorithm
{
/** 
 * @brief xxhash64加密数据
 * @param data 输入数据
 * @param dataLen 数据长度
 * @return 哈希值
 */
uint64_t xxhash64Sign(const char* data, size_t dataLen);

/** 
 * @brief xxhash64加密数据列表
 * @param dataList 数据列表
 * @param sortFlag 是否对列表进行排序: 0-不排序, 1-从小到大排序, 2-从大到小排序
 * @return 哈希值
 */
uint64_t xxhash64SignList(std::vector<std::string> dataList, int sortFlag = 0);

/** 
 * @brief xxhash64加密文件
 * @param handle 文件句柄
 * @param blockSize 每次读取的文件块大小(字节), 最小1024字节
 * @param stopFunc 停止函数, 返回值: true-停止, false-继续
 * @return 哈希值
 */
uint64_t xxhash64SignFileHandle(FILE* handle, size_t blockSize = 1024, const std::function<bool()>& stopFunc = nullptr);

/** 
 * @brief xxhash64加密文件
 * @param filename 文件路径
 * @param blockSize 每次读取的文件块大小(字节), 最小1024字节
 * @param stopFunc 停止函数, 返回值: true-停止, false-继续
 * @return 哈希值
 */
uint64_t xxhash64SignFile(const std::string& filename, size_t blockSize = 1024, const std::function<bool()>& stopFunc = nullptr);
} // namespace algorithm
