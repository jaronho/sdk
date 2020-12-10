#include "module_manager.h"
#include <assert.h>
#include <chrono>
#include <iostream>

ModuleManager& ModuleManager::getInstance()
{
	static ModuleManager s_instance;
	return s_instance;
}

bool ModuleManager::registerCreator(const std::type_info& type, const Creator& creator)
{
	std::cout << "register module [" << type.name() << "] creator\n";
	assert(!m_creators[type]);
	m_creators[type] = creator;
	return true;
}

int ModuleManager::create()
{
	for (const auto& creatorIter : m_creators)
	{
		std::cout << "creating module [" << creatorIter.first.name() << "]\n";
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		get(creatorIter.first, true);
		std::chrono::nanoseconds nanosecondsCost = std::chrono::steady_clock::now() - begin;
		std::chrono::milliseconds millisecondsCost = std::chrono::duration_cast<std::chrono::milliseconds>(nanosecondsCost);
		std::cout << "module [" << creatorIter.first.name() << "] created, cost " << millisecondsCost.count() << " ms\n";
	}
	return static_cast<int>(m_modules.size());
}

void ModuleManager::start()
{
	for (const auto& moduleIter : m_modules)
	{
		std::cout << "starting module [" << moduleIter.first.name() << "]\n";
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		moduleIter.second->onStart();
		std::chrono::nanoseconds nanosecondsCost = std::chrono::steady_clock::now() - begin;
		std::chrono::milliseconds millisecondsCost = std::chrono::duration_cast<std::chrono::milliseconds>(nanosecondsCost);
		std::cout << "module [" << moduleIter.first.name() << "] started, cost " << millisecondsCost.count() << " ms\n";
	}
}

void ModuleManager::resume()
{
	for (const auto& moduleIter : m_modules)
	{
		moduleIter.second->onResume();
	}
}

void ModuleManager::pause()
{
	for (const auto& moduleIter : m_modules)
	{
		moduleIter.second->onPause();
	}
}

void ModuleManager::stop()
{
	for (const auto& moduleIter : m_modules)
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

std::shared_ptr<Module> ModuleManager::get(const std::type_index& type, bool allowCreate)
{
	/* 先在已经创建的模块中搜索 */
	const auto moduleIter = m_modules.find(type);
	if (moduleIter != m_modules.end())
	{
		if (!moduleIter->second)
		{
			/* 从module列表中取出来是空(大概率不会出现) */
			std::cout << "module [" << type.name() << "] is nullptr\n";
			return nullptr;
		}
		return moduleIter->second;
	}
	/* 是否允许自动创建 */
	if (!allowCreate)
	{
		/* 模块未初始化时，中间层接口调用报错 */
		std::cout << "module [" << type.name() << "] not init\n";
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
			std::cout << "module [" << type.name() << "] create fail\n";
			return nullptr;
		}
		/* 创建后添加到列表中 */
		m_modules[type] = module;
		return module;
	}
	/* 模块不存在时报错 */
	std::cout << "module [" << type.name() << "] not found\n";
	return nullptr;
}
