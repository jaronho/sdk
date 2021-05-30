#pragma once

#include <map>
#include <memory>
#include <string>

#include "ini_writer.h"

#define INI_KEY_VAR(var) var
#define MAKE_INI_KV(id, keyVar, keyName, value, sectionComment, comment) \
    const std::string INI_KEY_VAR(keyVar) = ini::makeKeyValue(id, keyName, value, sectionComment, comment);

namespace ini
{
/**
 * @brief 分割section和key
 * @param sectionKey section和key的字符串组合, 格式为: "/section/key" 或 "/key"
 * @param sectionName [输出]section名称
 * @param key [输出]key名称
 */
void splitSectionKey(const std::string& sectionKey, std::string& sectionName, std::string& key);

/**
 * @brief 创建键值对
 * @param id 键值对所在模块id(一般传入文件名, 能够标识唯一即可)
 * @param sectionKey section和key的字符串组合, 格式为: "/section/key" 或 "/key"
 * @param value 值
 * @param sectionComment 节注释
 * @param comment 值注释
 * @return 返回sectionKey
 */
std::string makeKeyValue(const std::string& id, const std::string& sectionKey, const std::string& value,
                         const std::string& sectionComment = std::string(), const std::string& comment = std::string());

/**
 * @brief 恢复指定模块的配置
 * @param writer 写入器
 * @param id 键值对所在模块id(一般传入文件名, 能够标识唯一即可)
 * @return 0-成功, 1-写入器为空或不允许写入新字段, 2-找不到模块id, 3-保存失败
 */
int restoreIni(std::shared_ptr<IniWriter> writer, const std::string& id);

/**
 * @brief 同步指定模块的配置(将对写入器进行删除和增加键值, 不修改共有的键值)
 * @param writer 写入器
 * @param id 键值对所在模块id(一般传入文件名, 能够标识唯一即可)
 * @return 0-成功, 1-写入器为空或不允许写入新字段, 2-找不到模块id, 3-保存失败
 */
int syncIni(std::shared_ptr<IniWriter> writer, const std::string& id);
} // namespace ini
