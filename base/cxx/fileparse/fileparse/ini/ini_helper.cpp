#include "ini_helper.h"

#include <algorithm>
#include <mutex>
#include <stdexcept>

namespace ini
{
using INI_MAP_TYPE = std::map<std::string, std::map<std::string, IniSection>>;
std::mutex* g_mutex = nullptr; /* 互斥锁 */
INI_MAP_TYPE* g_iniMap = nullptr; /* 全局ini映射表 */

void splitSectionKey(const std::string& sectionKey, std::string& sectionName, std::string& key)
{
    if (sectionKey.size() < 2) /* 键值长度需要小于2 */
    {
        throw std::exception(std::logic_error("arg 'sectionKey.size' < 2"));
    }
    if ('/' != sectionKey[0] || '/' == sectionKey[sectionKey.size() - 1]) /* 首个字符不为/或最后一个字符为/ */
    {
        throw std::exception(std::logic_error("arg 'sectionKey' format error"));
    }
    auto count = std::count(sectionKey.begin(), sectionKey.end(), '/');
    if (count > 2) /* 超过两级 */
    {
        throw std::exception(std::logic_error("arg 'sectionKey' layer > 2"));
    }
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
                         const std::string& comment, bool readOnly)
{
    std::string sectionName, key;
    splitSectionKey(sectionKey, sectionName, key);
    /* 查找ini映射表 */
    if (!g_mutex)
    {
        g_mutex = new std::mutex();
    }
    std::lock_guard<std::mutex> locker(*g_mutex);
    if (!g_iniMap)
    {
        g_iniMap = new INI_MAP_TYPE();
    }
    auto iniIter = g_iniMap->find(id);
    if (g_iniMap->end() == iniIter) /* 没有ini配置则创建 */
    {
        iniIter = g_iniMap->insert(std::make_pair(id, std::map<std::string, IniSection>())).first;
    }
    auto sectionIter = iniIter->second.find(sectionName);
    if (iniIter->second.end() == sectionIter) /* 没有节则创建 */
    {
        IniSection section;
        section.name = sectionName;
        sectionIter = iniIter->second.insert(std::make_pair(sectionName, section)).first;
    }
    auto& section = sectionIter->second;
    /* 添加键值 */
    if (!sectionComment.empty())
    {
        if ('#' == sectionComment[0])
        {
            section.comment = sectionComment;
        }
        else
        {
            section.comment = "# " + sectionComment;
        }
    }
    for (size_t i = 0; i < section.items.size(); ++i)
    {
        if (0 == key.compare(section.items[i].key)) /* 键值已存在则返回 */
        {
            return sectionKey;
        }
    }
    /* 插入新键值 */
    IniItem item;
    item.key = key;
    item.value = value;
    if (!comment.empty())
    {
        if ('#' == comment[0])
        {
            item.comment = comment;
        }
        else
        {
            item.comment = "# " + comment;
        }
    }
    item.extra = readOnly ? "1" : "0"; /* 这里额外参数用作只读属性 */
    section.items.emplace_back(item);
    return sectionKey;
}

std::map<std::string, IniSection> getIni(const std::string& id)
{
    /* 查找ini映射表 */
    if (!g_mutex)
    {
        g_mutex = new std::mutex();
    }
    std::lock_guard<std::mutex> locker(*g_mutex);
    if (!g_iniMap)
    {
        return std::map<std::string, IniSection>();
    }
    auto iniIter = g_iniMap->find(id);
    if (g_iniMap->end() == iniIter) /* 没有ini配置则返回 */
    {
        return std::map<std::string, IniSection>();
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
    if (!g_mutex)
    {
        g_mutex = new std::mutex();
    }
    std::lock_guard<std::mutex> locker(*g_mutex);
    if (!g_iniMap)
    {
        return 2;
    }
    auto iniIter = g_iniMap->find(id);
    if (g_iniMap->end() == iniIter) /* 没有ini配置则返回 */
    {
        return 2;
    }
    /* 进行配置恢复 */
    writer->clear(); /* 先清空 */
    const auto& sectionMap = iniIter->second;
    for (auto sectionIter = sectionMap.begin(); sectionMap.end() != sectionIter; ++sectionIter)
    {
        const auto& section = sectionIter->second;
        if (!section.comment.empty())
        {
            writer->setSectionComment(section.name, section.comment);
        }
        for (size_t i = 0; i < section.items.size(); ++i)
        {
            const auto& item = section.items[i];
            writer->setValue(section.name, item.key, item.value);
            if (!item.comment.empty())
            {
                writer->setComment(section.name, item.key, item.comment);
            }
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
    if (!g_mutex)
    {
        g_mutex = new std::mutex();
    }
    std::lock_guard<std::mutex> locker(*g_mutex);
    if (!g_iniMap)
    {
        return 2;
    }
    auto iniIter = g_iniMap->find(id);
    if (g_iniMap->end() == iniIter) /* 没有ini配置则返回 */
    {
        return 2;
    }
    bool changed = false; /* 是否有修改 */
    const auto& newSectionMap = iniIter->second;
    /* 删除废弃数据 */
    auto oldSectionMap = writer->getSections();
    for (auto oldSectionIter = oldSectionMap.begin(); oldSectionMap.end() != oldSectionIter; ++oldSectionIter)
    {
        const auto& oldSection = oldSectionIter->second;
        auto newSectionIter = newSectionMap.find(oldSection.name);
        if (newSectionMap.end() == newSectionIter) /* 旧的节数据在新配置中不存在, 则表示可以删除 */
        {
            if (oldSection.name.empty() && oldSection.items.empty()) /* 全局节且无数据则忽略 */
            {
                continue;
            }
            writer->removeSection(oldSection.name);
            changed = true;
        }
        else /* 旧的节数据在新配置中存在 */
        {
            const auto& newSection = newSectionIter->second;
            std::string oldSectionComment;
            writer->getSectionComment(oldSection.name, oldSectionComment);
            if (0 != oldSectionComment.compare(newSection.comment))
            {
                writer->setSectionComment(oldSection.name, newSection.comment);
                changed = true;
            }
            /* 删除无用的键值数据 */
            for (size_t i = 0; i < oldSection.items.size(); ++i)
            {
                const auto& oldItem = oldSection.items[i];
                bool needRemove = true; /* 键值是否删除 */
                for (size_t j = 0; j < newSection.items.size(); ++j)
                {
                    const auto& newItem = newSection.items[j];
                    if (newItem.key == oldItem.key) /* 旧的键值在新配置中存在 */
                    {
                        needRemove = false;
                        if (0 == newItem.extra.compare("1")) /* 只读则使用新的键值 */
                        {
                            writer->setValue(oldSection.name, oldItem.key, newItem.value);
                            changed = true;
                        }
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
                if (needRemove) /* 旧的键值在新配置中不存在, 则表示删除 */
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
        writer->getSectionComment(newSection.name, oldSectionComment);
        if (0 != oldSectionComment.compare(newSection.comment))
        {
            writer->setSectionComment(newSection.name, newSection.comment);
            changed = true;
        }
        for (size_t i = 0; i < newSection.items.size(); ++i)
        {
            const auto& newItem = newSection.items[i];
            if (!writer->hasKey(newSection.name, newItem.key)) /* 新键值 */
            {
                writer->setValue(newSection.name, newItem.key, newItem.value);
                changed = true;
            }
            std::string oldComment;
            writer->getComment(newSection.name, newItem.key, oldComment);
            if (0 != oldComment.compare(newItem.comment))
            {
                writer->setComment(newSection.name, newItem.key, newItem.comment);
                changed = true;
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
