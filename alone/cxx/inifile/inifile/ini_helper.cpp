#include "ini_helper.h"

#include <algorithm>
#include <assert.h>
#include <mutex>

namespace ini
{
using INI_MAP_TYPE = std::map<std::string, std::map<std::string, std::shared_ptr<IniSection>>>;
std::mutex g_mutex;
INI_MAP_TYPE* g_iniMap = nullptr; /* 这里需要用原始指针(不要用共享指针) */

void splitSectionKey(const std::string& sectionKey, std::string& sectionName, std::string& key)
{
    assert(sectionKey.size() >= 2);
    assert('/' == sectionKey[0] && '/' != sectionKey[sectionKey.size() - 1]); /* 首个字符不为/或最后一个字符为/ */
    auto count = std::count(sectionKey.begin(), sectionKey.end(), '/');
    assert(count <= 2); /* 超过两级 */
    auto pos = sectionKey.find_last_of('/');
    if (0 == pos) /* 全局键值 */
    {
        key = sectionKey.substr(1, sectionKey.size() - 1);
    }
    else /* 二级键值 */
    {
        sectionName = sectionKey.substr(1, pos - 1);
        key = sectionKey.substr(pos + 1, sectionKey.size() - pos);
    }
}

std::string makeKeyValue(const std::string& id, const std::string& sectionKey, const std::string& value, const std::string& sectionComment,
                         const std::string& comment)
{
    std::string sectionName, key;
    splitSectionKey(sectionKey, sectionName, key);
    std::lock_guard<std::mutex> locker(g_mutex);
    /* 查找ini配置 */
    if (!g_iniMap)
    {
        g_iniMap = new INI_MAP_TYPE();
    }
    auto iniIter = g_iniMap->find(id);
    if (g_iniMap->end() == iniIter)
    {
        iniIter = g_iniMap->insert(std::make_pair(id, std::map<std::string, std::shared_ptr<IniSection>>())).first;
    }
    std::shared_ptr<IniSection> section;
    auto sectionIter = iniIter->second.find(sectionName);
    if (iniIter->second.end() == sectionIter) /* 没有则创建 */
    {
        section = std::make_shared<IniSection>();
        section->name = sectionName;
        iniIter->second.insert(std::make_pair(sectionName, section));
    }
    else
    {
        section = sectionIter->second;
    }
    /* 添加键值 */
    if (!sectionComment.empty())
    {
        section->comment = sectionComment;
    }
    for (size_t i = 0; i < section->items.size(); ++i)
    {
        if (0 == key.compare(section->items[i].key)) /* 键值重复 */
        {
            return sectionKey;
        }
    }
    IniItem item;
    item.key = key;
    item.value = value;
    item.comment = comment;
    section->items.emplace_back(item);
    return sectionKey;
}

int restoreIni(std::shared_ptr<IniWriter> writer, const std::string& id)
{
    if (!writer || !writer->isAllowAutoCreate())
    {
        return 1;
    }
    std::lock_guard<std::mutex> locker(g_mutex);
    /* 查找ini配置 */
    if (!g_iniMap)
    {
        g_iniMap = new INI_MAP_TYPE();
    }
    auto iniIter = g_iniMap->find(id);
    if (g_iniMap->end() == iniIter)
    {
        return 2;
    }
    /* 进行配置 */
    const auto& sectionMap = iniIter->second;
    writer->clear(); /* 先清空 */
    for (auto sectionIter = sectionMap.begin(); sectionMap.end() != sectionIter; ++sectionIter)
    {
        auto section = sectionIter->second;
        if (!section->comment.empty())
        {
            writer->setSectionComment(section->name, section->comment);
        }
        for (size_t i = 0; i < section->items.size(); ++i)
        {
            const auto& item = section->items[i];
            writer->setValue(section->name, item.key, item.value);
            if (!item.comment.empty())
            {
                writer->setComment(section->name, item.key, item.comment);
            }
        }
    }
    /* 最后要保存 */
    if (!writer->save())
    {
        return 3;
    }
    return 0;
}

int syncIni(std::shared_ptr<IniWriter> writer, const std::string& id)
{
    if (!writer || !writer->isAllowAutoCreate())
    {
        return 1;
    }
    std::lock_guard<std::mutex> locker(g_mutex);
    /* 查找ini配置 */
    if (!g_iniMap)
    {
        g_iniMap = new INI_MAP_TYPE();
    }
    auto iniIter = g_iniMap->find(id);
    if (g_iniMap->end() == iniIter)
    {
        return 2;
    }
    bool changed = false;
    const auto& newSectionMap = iniIter->second;
    /* 删除废弃数据 */
    auto oldSectionList = writer->getDataList();
    for (auto i = 0; i < oldSectionList.size(); ++i)
    {
        const auto& oldSection = oldSectionList[i];
        auto newSectionIter = newSectionMap.find(oldSection.name);
        if (newSectionMap.end() == newSectionIter) /* 删除无用的节数据 */
        {
            if (oldSection.name.empty() && oldSection.items.empty()) /* 全局节且无数据则忽略 */
            {
                continue;
            }
            writer->removeSection(oldSection.name);
            changed = true;
        }
        else
        {
            const auto& newSection = newSectionIter->second;
            std::string oldSectionComment;
            writer->getSectionComment(oldSection.name, oldSectionComment);
            if (0 != oldSectionComment.compare(newSection->comment))
            {
                writer->setSectionComment(oldSection.name, newSection->comment);
                changed = true;
            }
            /* 删除无用的键值数据 */
            for (size_t j = 0; j < oldSection.items.size(); ++j)
            {
                const auto& oldItem = oldSection.items[j];
                bool needRemove = true;
                for (size_t k = 0; k < newSection->items.size(); ++k)
                {
                    const auto& newItem = newSection->items[k];
                    if (newItem.key == oldItem.key)
                    {
                        needRemove = false;
                        std::string oldComment;
                        writer->getComment(oldSection.name, oldItem.key, oldComment);
                        if (0 != oldComment.compare(newItem.comment))
                        {
                            writer->setComment(oldSection.name, oldItem.key, newItem.comment);
                            changed = true;
                        }
                        break;
                    }
                }
                if (needRemove)
                {
                    writer->removeKey(oldSection.name, oldItem.key);
                    changed = true;
                }
            }
        }
    }
    /* 增加新数据 */
    for (auto newSectionIter = newSectionMap.begin(); newSectionMap.end() != newSectionIter; ++newSectionIter)
    {
        const auto& newSection = newSectionIter->second;
        std::string oldSectionComment;
        writer->getSectionComment(newSection->name, oldSectionComment);
        if (0 != oldSectionComment.compare(newSection->comment))
        {
            writer->setSectionComment(newSection->name, newSection->comment);
            changed = true;
        }
        for (size_t k = 0; k < newSection->items.size(); ++k)
        {
            const auto& newItem = newSection->items[k];
            if (!writer->hasKey(newSection->name, newItem.key))
            {
                writer->setValue(newSection->name, newItem.key, newItem.value);
                changed = true;
            }
            std::string oldComment;
            writer->getComment(newSection->name, newItem.key, oldComment);
            if (0 != oldComment.compare(newItem.comment))
            {
                writer->setComment(newSection->name, newItem.key, newItem.comment);
                changed = true;
            }
        }
    }
    /* 最后要保存 */
    if (changed)
    {
        if (!writer->save())
        {
            return 3;
        }
    }
    return 0;
}
} // namespace ini
