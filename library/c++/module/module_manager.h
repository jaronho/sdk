#ifndef __MODULE_MANAGER_H__
#define __MODULE_MANAGER_H__
#include <functional>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

/*
 * Brief: 所有模块的基类
 */
class Module
{
	friend class ModuleManager;
protected:
	/* 在初始化时统一构造(同步), 可以获取依赖模块的实例, 可以添加委托 */
	Module(void) {};

	/* 注销时统一释放(同步) */
	virtual ~Module(void) {};

	/* 模块启动(异步), 在所有模块构造完成之后调用, 一般用来加载最小资源 */
	virtual void onStart(void) {};

	/* 模块恢复(异步), 在应用进入前台运行时调用, 可以加载其他必要资源 */
	virtual void onResume(void) {};

	/* 模块挂起(异步), 在应用进入后台运行时调用, 应该主动释放定时器, 网络连接等资源, 实现省电 */
	virtual void onPause(void) {};

	/* 模块停止(异步), 在应用即将退出时调用, 应该释放'onStart'中加载的所有资源 */
	virtual void onStop(void) {};

private: /* noncopale */
	Module(const Module&) = default;
	Module& operator=(const Module&) = default;
};

/*
 * Brief: 模块管理器
 */
class ModuleManager final
{
	typedef std::function<std::shared_ptr<Module>()> Creator;

public:
	/*
	 * Brief:   获取模块管理器单例
	 * Param:   void
	 * Return:  单例对象
	 */
	static ModuleManager& getInstance(void);

	/*
	 * Brief:   注册模块创建函数
	 * Param:   type - 模块类型id, 通过'typeid'获得
	 *          creator - 模块创建函数
	 * Return:  void
	 */
	bool registerCreator(const std::type_info& type, const Creator& creator);

	/*
	 * Brief:   根据模块类型获取模块实例, 没有找到则创建
	 * Param:   T - 获取的模块类型
	 *          allowCreate - 是否允许创建(不允许时,若未创建则返回nullptr)
	 * Return:  模块共享指针
	 */
	template<class T>
	std::shared_ptr<T> get(bool allowCreate = true)
	{
		const auto module = get(typeid(T), allowCreate);
		return std::dynamic_pointer_cast<T>(module);
	}

	/*
	 * Brief:   创建所有模块(同步)
	 * Param:   void
	 * Return:  模块数量
	 */
	int create(void);

	/*
	 * Brief:   启动所有模块(异步)
	 * Param:   void
	 * Return:  void
	 */
	void start(void);

	/*
	 * Brief:   恢复所有模块(异步)
	 * Param:   void
	 * Return:  void
	 */
	void resume(void);

	/*
	 * Brief:   挂起所有模块(异步)
	 * Param:   void
	 * Return:  void
	 */
	void pause(void);

	/*
	 * Brief:   停止所有模块(异步)
	 * Param:   void
	 * Return:  void
	 */
	void stop(void);

	/*
	 * Brief:   销毁所有模块(同步)
	 * Param:   void
	 * Return:  模块数量
	 */
	int destroy(void);

private:
	ModuleManager(void) = default;

private: /* noncopale */
	ModuleManager(const ModuleManager&) = default;
	ModuleManager& operator=(const ModuleManager&) = default;

private:
	/*
	 * Brief:   根据模块类型id获取模块实例, 没有找到则创建
	 * Param:   type - 获取的模块类型id
	 *          allowCreate - 是否允许创建(不允许时,若未创建则返回nullptr)
	 * Return:  模块共享指针
	 */
	std::shared_ptr<Module> get(const std::type_index& type, bool allowCreate = true);

private:
	std::unordered_map<std::type_index, Creator> m_creators; /* 模块类型id <-> 模块创建函数, 映射表 */
	std::unordered_map<std::type_index, std::shared_ptr<Module>> m_modules; /* 模块类型id <-> 模块, 映射表 */
};

/* 模块管理器 */
#define MODULE_MANAGER() ModuleManager::getInstance()

/* 注册模块, 应该在cpp文件中使用 */
#define REGISTER_MODULE(moduleType) \
    static bool s_module_##moduleType##_registered = ModuleManager::getInstance().registerCreator( \
        typeid(moduleType), \
        []() -> std::shared_ptr<Module> { \
            return std::make_shared<moduleType>(); \
        })

/* 注册模块, 应该在cpp文件中使用, 接口层使用虚接口时使用 */
#define REGISTER_MODULE_IMPL(moduleType, moduleTypeImpl) \
    static bool s_module_##moduleType##_registered = ModuleManager::getInstance().registerCreator( \
        typeid(moduleType), \
        []() -> std::shared_ptr<Module> { \
            return std::make_shared<moduleTypeImpl>(); \
        })

/* 获取模块 */
#define GET_MODULE(moduleType) ModuleManager::getInstance().get<moduleType>()

#endif /* __MODULE_MANAGER_H__ */

/*
************************************************** sample_01

#include "module_manager.h"

class Mod : public Module
{
public:
	void print()
	{
		std::cout << "--- Mod print\n";
	}
};
REGISTER_MODULE(Mod);

class Base : public Module
{
public:
	virtual void say() = 0;
};

class Child : public Base
{
public:
	virtual void say() override
	{
		std::cout << "--- Child say\n";
	}
};
REGISTER_MODULE_IMPL(Base, Child);

int main()
{
	ModuleManager::getInstance().create();
	ModuleManager::getInstance().start();

	GET_MODULE(Mod)->print();
	GET_MODULE(Base)->say();

	ModuleManager::getInstance().stop();
	ModuleManager::getInstance().destroy();
	return 0;
}

*/
