#pragma once
#include <functional>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace utilitiy
{
/**
 * @brief 所有模块的基类
 */
class Module
{
    friend class ModuleManager;

protected:
    /* 在初始化时统一构造(被同步调用), 可以获取依赖模块的实例, 可以添加委托 */
    Module(){};

    /* 注销时统一释放(被同步调用) */
    virtual ~Module(){};

    /* 模块启动(一般被异步调用), 在所有模块构造完成之后调用, 一般用来加载最小资源 */
    virtual void onStart(){};

    /* 模块恢复(一般被异步调用), 在应用进入前台运行时调用, 可以加载其他必要资源 */
    virtual void onResume(){};

    /* 模块挂起(一般被异步调用), 在应用进入后台运行时调用, 应该主动释放定时器, 网络连接等资源, 实现省电 */
    virtual void onPause(){};

    /* 模块停止(一般被异步调用), 在应用即将退出时调用, 应该释放'onStart'中加载的所有资源 */
    virtual void onStop(){};

private: /* noncopale */
    Module(const Module&) = default;
    Module& operator=(const Module&) = default;
};

/**
 * @brief 模块管理器
 */
class ModuleManager final
{
    typedef std::function<std::shared_ptr<Module>()> Creator;

public:
    /**
	 * @brief 获取模块管理器单例
	 * @return 单例对象
	 */
    static ModuleManager& getInstance();

    /**
	 * @brief 设置日志函数
	 * @param logFunc 日志函数
	 */
    void setLogFunc(const std::function<void(const std::string&)>& logFunc);

    /**
	 * @brief 注册模块创建函数
	 * @param type 模块类型id, 通过'typeid'获得
	 * @param creator 模块创建函数
	 */
    bool registerCreator(const std::type_info& type, const Creator& creator);

    /**
	 * @brief 根据模块类型获取模块实例, 没有找到则创建
	 * @param T 获取的模块类型
	 * @param allowCreate 是否允许创建(不允许时,若未创建则返回nullptr)
	 * @return 模块共享指针
	 */
    template<class T>
    std::shared_ptr<T> get(bool allowCreate = true)
    {
        const auto utilitiy = get(typeid(T), allowCreate);
        return std::dynamic_pointer_cast<T>(utilitiy);
    }

    /**
	 * @brief 创建所有模块(必须同步调用)
	 * @return 模块数量
	 */
    int create();

    /**
	 * @brief 启动所有模块(建议异步调用)
	 */
    void start();

    /**
	 * @brief 恢复所有模块(建议异步调用)
	 */
    void resume();

    /**
	 * @brief 挂起所有模块(建议异步调用)
	 */
    void pause();

    /**
	 * @brief 停止所有模块(建议异步调用)
	 */
    void stop();

    /**
	 * @brief 销毁所有模块(必须同步调用)
	 * @return 模块数量
	 */
    int destroy();

private:
    ModuleManager() = default;

private: /* noncopale */
    ModuleManager(const ModuleManager&) = default;
    ModuleManager& operator=(const ModuleManager&) = default;

private:
    /**
	 * @brief 根据模块类型id获取模块实例, 没有找到则创建
	 * @param type 获取的模块类型id
	 * @param allowCreate 是否允许创建(不允许时,若未创建则返回nullptr)
	 * @return 模块共享指针
	 */
    std::shared_ptr<Module> get(const std::type_index& type, bool allowCreate = true);

    /**
	 * @brief 打印日志信息
	 * @param msg 日志消息
	 */
    void printLog(const std::string& msg);

private:
    std::unordered_map<std::type_index, Creator> m_creators; /* 模块类型id <-> 模块创建函数, 映射表 */
    std::unordered_map<std::type_index, std::shared_ptr<Module>> m_modules; /* 模块类型id <-> 模块, 映射表 */
    std::function<void(const std::string&)> m_logFunc = nullptr; /* 日志函数 */
};
} // namespace utilitiy

/* 模块管理器 */
#define MODULE_MANAGER() utilitiy::ModuleManager::getInstance()

/* 注册模块, 只能在cpp文件中使用 */
#define REGISTER_MODULE(moduleType) \
    static bool s_module_##moduleType##_registered = utilitiy::ModuleManager::getInstance().registerCreator( \
        typeid(moduleType), []() -> std::shared_ptr<utilitiy::Module> { return std::make_shared<moduleType>(); })

/* 注册模块, 只能在cpp文件中使用 */
#define REGISTER_MODULE_IMPL(moduleType, moduleTypeImpl) \
    static bool s_module_##moduleType##_registered = utilitiy::ModuleManager::getInstance().registerCreator( \
        typeid(moduleType), []() -> std::shared_ptr<utilitiy::Module> { return std::make_shared<moduleTypeImpl>(); })

/* 获取模块 */
#define GET_MODULE(moduleType) utilitiy::ModuleManager::getInstance().get<moduleType>()
