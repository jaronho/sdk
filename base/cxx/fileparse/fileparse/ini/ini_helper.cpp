#include "ini_helper.h"

#include <algorithm>
#include <mutex>
#include <stdexcept>

namespace ini
{
static std::mutex s_mutex; /* 互斥锁 */
static std::unordered_map<std::string, std::unordered_map<std::string, ini::IniSection>> s_iniMap; /* 全局ini映射表 */

void splitSectionKey(const std::string& sectionKey, std::string& sectionName, std::string& keyName)
{
    sectionName.clear();
    keyName.clear();
    if (sectionKey.size() < 2) /* 键值长度需要小于2 */
    {
        throw std::logic_error("arg 'sectionKey' '" + sectionKey + "' size < 2");
    }
    if ('/' != sectionKey[0] || '/' == sectionKey[sectionKey.size() - 1]) /* 首个字符不为/或最后一个字符为/ */
    {
        throw std::logic_error("arg 'sectionKey' '" + sectionKey + "' format error");
    }
    auto count = std::count(sectionKey.begin(), sectionKey.end(), '/');
    if (count > 2) /* 超过两级 */
    {
        throw std::logic_error("arg 'sectionKey' '" + sectionKey + "' layer > 2");
    }
    auto pos = sectionKey.find_last_of('/');
    if (0 == pos) /* 全局键值 */
    {
        keyName = sectionKey.substr(1, sectionKey.size() - 1);
    }
    else /* 二级键值 */
    {
        sectionName = sectionKey.substr(1, pos - 1);
        keyName = sectionKey.substr(pos + 1, sectionKey.size() - pos);
    }
}

std::string makeKeyValue(const std::string& id, const std::string& sectionKey, const std::string& value, const std::string& sectionComment,
                         const std::string& keyComment, bool readOnly)
{
    std::string sectionName, key;
    splitSectionKey(sectionKey, sectionName, key);
    /* 查找ini映射表 */
    std::lock_guard<std::mutex> locker(s_mutex);
    auto iniIter = s_iniMap.find(id);
    if (s_iniMap.end() == iniIter) /* 没有ini配置则创建 */
    {
        iniIter = s_iniMap.insert(std::make_pair(id, std::unordered_map<std::string, IniSection>())).first;
    }
    /* 查找节 */
    auto sectionIter = iniIter->second.find(sectionName);
    if (iniIter->second.end() == sectionIter) /* 没有节则创建 */
    {
        ini::IniSection section;
        section.name = sectionName;
        sectionIter = iniIter->second.insert(std::make_pair(sectionName, section)).first;
    }
    auto& section = sectionIter->second;
    /* 添加键值 */
    for (const auto& item : section.items)
    {
        if (key == item.key) /* 键值已存在则返回 */
        {
            return sectionKey;
        }
    }
    if (!sectionComment.empty())
    {
        section.comment = ('#' == sectionComment[0]) ? sectionComment : ("# " + sectionComment);
    }
    /* 插入新键值 */
    IniItem item;
    item.key = key;
    item.value = value;
    if (!keyComment.empty())
    {
        item.comment = ('#' == keyComment[0]) ? keyComment : ("# " + keyComment);
    }
    item.extraMap["readOnly"] = readOnly ? "1" : "0";
    section.items.emplace_back(item);
    return sectionKey;
}

std::unordered_map<std::string, IniSection> getIni(const std::string& id)
{
    /* 查找ini映射表 */
    std::lock_guard<std::mutex> locker(s_mutex);
    auto iniIter = s_iniMap.find(id);
    if (s_iniMap.end() == iniIter) /* 没有ini配置则返回 */
    {
        return std::unordered_map<std::string, IniSection>();
    }
    return iniIter->second;
}

int restoreIni(std::shared_ptr<IniWriter> writer, const std::string& id, bool autoSave)
{
    if (!writer || !writer->isAllowAutoCreate())
    {
        return 1;
    }
    /* 查找ini映射表 */
    std::lock_guard<std::mutex> locker(s_mutex);
    auto iniIter = s_iniMap.find(id);
    if (s_iniMap.end() == iniIter) /* 没有ini配置则返回 */
    {
        return 2;
    }
    /* 清空当前配置 */
    writer->clear();
    /* 恢复初始配置 */
    const auto& sectionMap = iniIter->second;
    for (auto sectionIter = sectionMap.begin(); sectionMap.end() != sectionIter; ++sectionIter)
    {
        const auto& section = sectionIter->second;
        if (!section.comment.empty())
        {
            writer->setSectionComment(section.name, section.comment);
        }
        for (const auto& item : section.items)
        {
            if (!item.comment.empty())
            {
                writer->setComment(section.name, item.key, item.comment);
            }
            writer->setValue(section.name, item.key, item.value);
        }
    }
    if (autoSave && !writer->save())
    {
        return 3;
    }
    return 0;
}

int syncIni(std::shared_ptr<IniWriter> writer, const std::string& id, bool autoSave)
{
    if (!writer || !writer->isAllowAutoCreate())
    {
        return 1;
    }
    /* 查找ini映射表 */
    std::lock_guard<std::mutex> locker(s_mutex);
    auto iniIter = s_iniMap.find(id);
    if (s_iniMap.end() == iniIter) /* 没有ini配置则返回 */
    {
        return 2;
    }
    bool changed = false; /* 是否有修改 */
    const auto& sectionMap = iniIter->second;
    /* 删除/修改配置 */
    auto localSectionMap = writer->getSections();
    for (auto localSectionIter = localSectionMap.begin(); localSectionMap.end() != localSectionIter; ++localSectionIter)
    {
        const auto& localSection = localSectionIter->second;
        auto sectionIter = sectionMap.find(localSection.name);
        if (sectionMap.end() == sectionIter) /* 本地节数据在配置中不存在, 则删除 */
        {
            writer->removeSection(localSection.name);
            changed = true;
        }
        else /* 本地节数据在配置中存在 */
        {
            const auto& section = sectionIter->second;
            std::string localSectionComment;
            writer->getSectionComment(localSection.name, localSectionComment);
            if (section.comment != localSectionComment)
            {
                if (0 == writer->setSectionComment(localSection.name, section.comment))
                {
                    changed = true;
                }
            }
            /* 删除无用的键值数据 */
            for (const auto& localItem : localSection.items)
            {
                const auto& itemIter = std::find_if(section.items.begin(), section.items.end(),
                                                    [&](const ini::IniItem& item) { return (item.key == localItem.key); });
                if (section.items.end() == itemIter) /* 本地键值在配置中不存在, 则删除 */
                {
                    writer->removeKey(localSection.name, localItem.key);
                    changed = true;
                }
                else /* 本地键值在配置中存在 */
                {
                    std::string localComment;
                    writer->getComment(localSection.name, localItem.key, localComment);
                    if (itemIter->comment != localComment)
                    {
                        if (0 == writer->setComment(localSection.name, localItem.key, itemIter->comment))
                        {
                            changed = true;
                        }
                    }
                    if (itemIter->value != localItem.value) /* 键值不相等 */
                    {
                        auto extraIter = itemIter->extraMap.find("readOnly");
                        if (itemIter->extraMap.end() != extraIter && "1" == extraIter->second) /* 只读则使用新键值 */
                        {
                            if (0 == writer->setValue(localSection.name, localItem.key, itemIter->value))
                            {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    /* 增加配置 */
    for (auto sectionIter = sectionMap.begin(); sectionMap.end() != sectionIter; ++sectionIter)
    {
        const auto& section = sectionIter->second;
        std::string localSectionComment;
        writer->getSectionComment(section.name, localSectionComment);
        if (section.comment != localSectionComment)
        {
            if (0 == writer->setSectionComment(section.name, section.comment))
            {
                changed = true;
            }
        }
        for (const auto& item : section.items)
        {
            if (!writer->hasKey(section.name, item.key)) /* 新键值 */
            {
                if (0 == writer->setComment(section.name, item.key, item.comment))
                {
                    changed = true;
                }
                if (0 == writer->setValue(section.name, item.key, item.value))
                {
                    changed = true;
                }
            }
        }
    }
    /* 自动保存 */
    if (changed && autoSave && !writer->save())
    {
        return 3;
    }
    return 0;
}
} // namespace ini
