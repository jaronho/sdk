#include "module_manager.h"
#include <assert.h>
#include <chrono>
#include <iostream>

ModuleManager &ModuleManager::getInstance()
{
    static ModuleManager s_instance;
    return s_instance;
}

void ModuleManager::setLogFunc(const std::function<void(const std::string &)> &logFunc)
{
    if (logFunc)
    {
        m_logFunc = logFunc;
    }
}

bool ModuleManager::registerCreator(const std::type_info &type, const Creator &creator)
{
    printLog("register module [" + std::string(type.name()) + "] creator");
    assert(!m_creators[type]);
    m_creators[type] = creator;
    return true;
}

int ModuleManager::create()
{
    for (const auto &creatorIter : m_creators)
    {
        printLog("creating module [" + std::string(creatorIter.first.name()) + "]");
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        get(creatorIter.first, true);
        std::chrono::nanoseconds nanosecondsCost = std::chrono::steady_clock::now() - begin;
        std::chrono::milliseconds millisecondsCost = std::chrono::duration_cast<std::chrono::milliseconds>(nanosecondsCost);
        printLog("module [" + std::string(creatorIter.first.name()) + "] created, cost " + std::to_string(millisecondsCost.count()) + " ms");
    }
    return static_cast<int>(m_modules.size());
}

void ModuleManager::start()
{
    for (const auto &moduleIter : m_modules)
    {
        printLog("starting module [" + std::string(moduleIter.first.name()) + "]");
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        moduleIter.second->onStart();
        std::chrono::nanoseconds nanosecondsCost = std::chrono::steady_clock::now() - begin;
        std::chrono::milliseconds millisecondsCost = std::chrono::duration_cast<std::chrono::milliseconds>(nanosecondsCost);
        printLog("module [" + std::string(moduleIter.first.name()) + "] started, cost " + std::to_string(millisecondsCost.count()) + " ms");
    }
}

void ModuleManager::resume()
{
    for (const auto &moduleIter : m_modules)
    {
        moduleIter.second->onResume();
    }
}

void ModuleManager::pause()
{
    for (const auto &moduleIter : m_modules)
    {
        moduleIter.second->onPause();
    }
}

void ModuleManager::stop()
{
    for (const auto &moduleIter : m_modules)
    {
        moduleIter.second->onStop();
    }
}

int ModuleManager::destroy()
{
    const auto size = m_modules.size();
    m_modules.clear();
    return static_cast<int>(size);
}

std::shared_ptr<Module> ModuleManager::get(const std::type_index &type, bool allowCreate)
{
    /* 先在已经创建的模块中搜索 */
    const auto moduleIter = m_modules.find(type);
    if (moduleIter != m_modules.end())
    {
        if (!moduleIter->second)
        {
            /* 从module列表中取出来是空(大概率不会出现) */
            printLog("module [" + std::string(type.name()) + "] is nullptr");
            return nullptr;
        }
        return moduleIter->second;
    }
    /* 是否允许自动创建 */
    if (!allowCreate)
    {
        /* 模块未初始化 */
        printLog("module [" + std::string(type.name()) + "] not init");
        return nullptr;
    }
    /* 没有找到则创建 */
    const auto creatorIter = m_creators.find(type);
    if (creatorIter != m_creators.end())
    {
        auto module = creatorIter->second();
        if (!module)
        {
            /* creator返回了空指针 */
            printLog("module [" + std::string(type.name()) + "] create fail");
            return nullptr;
        }
        /* 创建后添加到列表中 */
        m_modules[type] = module;
        return module;
    }
    /* 模块不存在时报错 */
    printLog("module [" + std::string(type.name()) + "] not found");
    return nullptr;
}

void ModuleManager::printLog(const std::string &msg)
{
    if (m_logFunc)
    {
        m_logFunc(msg);
    }
    else
    {
        std::cout << msg + "\n";
    }
}
