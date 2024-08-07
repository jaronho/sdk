#include "ini_helper.h"

#include <algorithm>
#include <mutex>
#include <stdexcept>
#include <vector>

namespace ini
{
static std::mutex s_mutex; /* 互斥锁 */
static std::map<std::string, std::vector<ini::IniSection>> s_iniMap; /* 全局ini映射表 */

void splitSectionKey(const std::string& sectionKey, std::string& name, std::string& key)
{
    name.clear();
    key.clear();
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
        key = sectionKey.substr(1, sectionKey.size() - 1);
    }
    else /* 二级键值 */
    {
        name = sectionKey.substr(1, pos - 1);
        key = sectionKey.substr(pos + 1, sectionKey.size() - pos);
    }
}

std::string makeKeyValue(const std::string& id, const std::string& sectionKey, const std::string& value, const std::string& sectionComment,
                         const std::string& comment, bool readOnly)
{
    std::string name, key;
    splitSectionKey(sectionKey, name, key);
    /* 查找ini映射表 */
    std::lock_guard<std::mutex> locker(s_mutex);
    auto iniIter = s_iniMap.find(id);
    if (s_iniMap.end() == iniIter) /* 没有ini配置则创建 */
    {
        iniIter = s_iniMap.insert(std::make_pair(id, std::vector<IniSection>())).first;
    }
    /* 查找节 */
    auto sectionIter = iniIter->second.begin();
    for (; iniIter->second.end() != sectionIter; ++sectionIter)
    {
        if (name == sectionIter->name)
        {
            break;
        }
    }
    if (iniIter->second.end() == sectionIter) /* 没有节则创建 */
    {
        ini::IniSection section;
        section.name = name;
        sectionIter = iniIter->second.insert(iniIter->second.end(), section);
    }
    /* 添加项 */
    for (const auto& item : sectionIter->items)
    {
        if (key == item.key) /* 项已存在则返回 */
        {
            return sectionKey;
        }
    }
    if (!sectionComment.empty())
    {
        sectionIter->comment = ('#' == sectionComment[0]) ? sectionComment : ("# " + sectionComment);
    }
    /* 插入新项 */
    IniItem item;
    item.key = key;
    item.value = value;
    if (!comment.empty())
    {
        item.comment = ('#' == comment[0]) ? comment : ("# " + comment);
    }
    item.extraMap["readOnly"] = readOnly ? "1" : "0";
    sectionIter->items.emplace_back(item);
    return sectionKey;
}

std::vector<IniSection> getIni(const std::string& id)
{
    /* 查找ini映射表 */
    std::lock_guard<std::mutex> locker(s_mutex);
    auto iniIter = s_iniMap.find(id);
    if (s_iniMap.end() == iniIter) /* 没有ini配置则返回 */
    {
        return std::vector<IniSection>();
    }
    return iniIter->second;
}

int restoreIni(std::shared_ptr<IniWriter> writer, const std::string& id, const std::vector<std::string>& ignoreSections,
               const std::vector<std::string>& ignoreItems, bool autoSave, int sortType)
{
    if (!writer)
    {
        return 2;
    }
    /* 查找ini映射表 */
    std::lock_guard<std::mutex> locker(s_mutex);
    auto iniIter = s_iniMap.find(id);
    if (s_iniMap.end() == iniIter) /* 没有ini配置则返回 */
    {
        return 3;
    }
    /* 判断节数据是否要忽略 */
    auto isSectionIgnored = [&](const std::string& name) {
        for (const auto& ignoreSection : ignoreSections)
        {
            if (name == ignoreSection)
            {
                return 0; /* 完全忽略 */
            }
        }
        for (const auto& ignoreItem : ignoreItems)
        {
            std::string sn, ik;
            splitSectionKey(ignoreItem, sn, ik);
            if (name == sn)
            {
                return 1; /* 不完全忽略 */
            }
        }
        return 2; /* 不忽略 */
    };
    /* 判断项数据是否要忽略 */
    auto isItemIgnored = [&](const std::string& name, const std::string& key) {
        for (const auto& ignoreItem : ignoreItems)
        {
            std::string sn, ik;
            splitSectionKey(ignoreItem, sn, ik);
            if (name == sn && key == ik)
            {
                return true;
            }
        }
        return false;
    };
    /* 清空当前配置 */
    auto sections = writer->getSections();
    for (const auto& section : sections)
    {
        auto sectionIter = iniIter->second.begin();
        for (; iniIter->second.end() != sectionIter; ++sectionIter)
        {
            if (section.name == sectionIter->name)
            {
                break;
            }
        }
        if (iniIter->second.end() == sectionIter) /* 节数据不存在 */
        {
            auto ret = isSectionIgnored(section.name);
            if (1 == ret) /* 不完全忽略节(只忽略部分项) */
            {
                for (const auto& item : section.items)
                {
                    if (!isItemIgnored(section.name, item.key)) /* 不忽略, 则丢弃 */
                    {
                        writer->removeItem(section.name, item.key);
                    }
                }
            }
            else if (2 == ret) /* 不忽略, 则丢弃 */
            {
                writer->removeSection(section.name);
            }
        }
        else /* 节数据存在 */
        {
            for (const auto& item : section.items)
            {
                auto itemIter = sectionIter->items.begin();
                for (; sectionIter->items.end() != itemIter; ++itemIter)
                {
                    if (item.key == itemIter->key)
                    {
                        break;
                    }
                }
                if (sectionIter->items.end() == itemIter) /* 项数据不存在 */
                {
                    if (!isItemIgnored(section.name, item.key)) /* 不忽略, 则丢弃 */
                    {
                        writer->removeItem(section.name, item.key);
                    }
                }
            }
        }
    }
    /* 恢复初始配置 */
    for (const auto& section : iniIter->second)
    {
        if (0 == isSectionIgnored(section.name))
        {
            continue;
        }
        if (!section.name.empty())
        {
            if (writer->setSectionComment(section.name, section.comment) >= 2)
            {
                return 4;
            }
        }
        for (const auto& item : section.items)
        {
            if (isItemIgnored(section.name, item.key))
            {
                continue;
            }
            if (writer->setComment(section.name, item.key, item.comment) >= 2)
            {
                return 4;
            }
            if (writer->setValue(section.name, item.key, item.value) >= 2)
            {
                return 4;
            }
        }
    }
    if (autoSave)
    {
        auto ret = writer->save(sortType);
        if (0 != ret)
        {
            return (1 == ret) ? 1 : 5;
        }
    }
    return 0;
}

int syncIni(std::shared_ptr<IniWriter> writer, const std::string& id, bool rmFlag, bool autoSave, int sortType)
{
    if (!writer)
    {
        return 2;
    }
    /* 查找ini映射表 */
    std::lock_guard<std::mutex> locker(s_mutex);
    auto iniIter = s_iniMap.find(id);
    if (s_iniMap.end() == iniIter) /* 没有ini配置则返回 */
    {
        return 3;
    }
    /* 删除/修改配置 */
    auto sections = writer->getSections();
    for (const auto& section : sections)
    {
        auto sectionIter = iniIter->second.begin();
        for (; iniIter->second.end() != sectionIter; ++sectionIter)
        {
            if (section.name == sectionIter->name)
            {
                break;
            }
        }
        if (iniIter->second.end() == sectionIter) /* 节数据不存在, 则丢弃 */
        {
            if (rmFlag)
            {
                writer->removeSection(section.name);
            }
        }
        else /* 节数据存在 */
        {
            std::string sectionComment;
            writer->getSectionComment(section.name, sectionComment);
            if (sectionComment != sectionIter->comment)
            {
                if (writer->setSectionComment(section.name, sectionIter->comment) >= 2)
                {
                    return 4;
                }
            }
            for (const auto& item : section.items)
            {
                auto itemIter = sectionIter->items.begin();
                for (; sectionIter->items.end() != itemIter; ++itemIter)
                {
                    if (item.key == itemIter->key)
                    {
                        break;
                    }
                }
                if (sectionIter->items.end() == itemIter) /* 项数据不存在, 则丢弃 */
                {
                    if (rmFlag)
                    {
                        writer->removeItem(section.name, item.key);
                    }
                }
                else /* 项数据存在 */
                {
                    std::string comment;
                    writer->getComment(section.name, item.key, comment);
                    if (comment != itemIter->comment)
                    {
                        if (writer->setComment(section.name, item.key, itemIter->comment) >= 2)
                        {
                            return 4;
                        }
                    }
                    if (item.value != itemIter->value) /* 值不相等 */
                    {
                        auto extraIter = itemIter->extraMap.find("readOnly");
                        if (itemIter->extraMap.end() != extraIter && "1" == extraIter->second) /* 只读则使用新项 */
                        {
                            if (writer->setValue(section.name, item.key, itemIter->value) >= 2)
                            {
                                return 4;
                            }
                        }
                    }
                }
            }
        }
    }
    /* 增加配置 */
    for (const auto& section : iniIter->second)
    {
        std::string sectionComment;
        writer->getSectionComment(section.name, sectionComment);
        if (sectionComment != section.comment)
        {
            if (writer->setSectionComment(section.name, section.comment) >= 2)
            {
                return 4;
            }
        }
        for (const auto& item : section.items)
        {
            if (!writer->hasItem(section.name, item.key)) /* 新项 */
            {
                if (writer->setComment(section.name, item.key, item.comment) >= 2)
                {
                    return 4;
                }
                if (writer->setValue(section.name, item.key, item.value) >= 2)
                {
                    return 4;
                }
            }
        }
    }
    /* 自动保存 */
    if (autoSave)
    {
        auto ret = writer->save(sortType);
        if (0 != ret)
        {
            return (1 == ret) ? 1 : 5;
        }
    }
    return 0;
}
} // namespace ini
