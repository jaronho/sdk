#pragma once
#include <map>
#include <memory>
#include <string>

#include "ini_writer.h"

#define INI_KEY_VAR(var) var
/* 创建字符串键值对 */
#define MAKE_INI_KV_STRING(id, var, key, value, sectionComment, comment, readOnly) \
    auto INI_KEY_VAR(var) = ini::makeKeyValue(id, key, value, sectionComment, comment, readOnly)
/* 创建数值键值对 */
#define MAKE_INI_KV_NUMBER(id, var, key, value, sectionComment, comment, readOnly) \
    auto INI_KEY_VAR(var) = ini::makeKeyValue(id, key, std::to_string(value), sectionComment, comment, readOnly)

namespace ini
{
/**
 * @brief 分割section和key
 * @param sectionKey section和key的字符串组合, 格式为: "/section/key" 或 "/key"
 * @param name [输出]section名称
 * @param key [输出]key名称
 */
void splitSectionKey(const std::string& sectionKey, std::string& name, std::string& key);

/**
 * @brief 创建键值对
 * @param id 键值对所在模块id(一般传入文件名, 能够标识唯一即可)
 * @param sectionKey section和key的字符串组合, 格式为: "/section/key" 或 "/key"
 * @param value 值
 * @param sectionComment 节注释(选填), 默认为空
 * @param comment 项注释(选填), 默认为空
 * @param readOnly 是否只读
 * @return 返回sectionKey
 */
std::string makeKeyValue(const std::string& id, const std::string& sectionKey, const std::string& value,
                         const std::string& sectionComment = std::string(), const std::string& comment = std::string(),
                         bool readOnly = false);

/**
 * @brief 获取指定模块配置
 * @param id 键值对所在模块id(一般传入文件名, 能够标识唯一即可)
 * @return 返回初始配置
 */
std::vector<IniSection> getIni(const std::string& id);

/**
 * @brief 恢复指定模块的配置
 * @param writer 读写器
 * @param id 键值对所在模块id(一般传入文件名, 能够标识唯一即可)
 * @param ignoreSections 要忽略的节数据(选填), 默认为空
 * @param ignoreItems 要忽略的项数据(选填), 默认为空
 * @param autoSave 恢复后是否自动保存(选填), 默认自动保存
 * @param sortType 排序类型(选填): 0-默认, 1-升序, 2-降序
 * @return 0-成功, 1-未变更, 2-读写器为空, 3-找不到模块id, 4-恢复失败,5-保存失败
 */
int restoreIni(std::shared_ptr<IniWriter> writer, const std::string& id, const std::vector<std::string>& ignoreSections = {},
               const std::vector<std::string>& ignoreItems = {}, bool autoSave = true, int sortType = 0);

/**
 * @brief 同步指定模块的配置(将对读写器进行删除和增加键值, 不修改共有的键值)
 * @param writer 读写器
 * @param id 键值对所在模块id(一般传入文件名, 能够标识唯一即可)
 * @param rmFlag 是否删除丢弃的节/项(选填), 默认删除
 * @param autoSave 同步后是否自动保存(选填), 默认自动保存
 * @param sortType 排序类型(选填): 0-默认, 1-升序, 2-降序
 * @return 0-成功, 1-未变更, 2-读写器为空, 3-找不到模块id, 4-同步失败, 5-保存失败
 */
int syncIni(std::shared_ptr<IniWriter> writer, const std::string& id, bool rmFlag = true, bool autoSave = true, int sortType = 0);
} // namespace ini
