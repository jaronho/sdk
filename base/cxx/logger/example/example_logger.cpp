#include <iostream>

#include "logger/logger_manager.h"

logger::Logger g_logger;

int main()
{
    logger::LogConfig cfg;
    cfg.path = "log";
    cfg.name = "app"; /* 默认的日志记录器名称 */
    cfg.fileExtName = ".log";
    cfg.fileMaxSize = 20 * 1024 * 1024;
    cfg.fileMaxCount = 5;
    cfg.newFolderDaily = true;
    logger::LoggerManager::start(cfg, "who");
    auto logger1 = logger::LoggerManager::getLogger("test", "main_");
    for (int i = 0; i < 100; ++i)
    {
        if (i <= 16)
        {
            TRACE_LOG(logger1, "hello world, I'm main, i[{}]", i);
        }
        else if (i <= 32)
        {
            DEBUG_LOG(logger1, "hello world, I'm main, i[{}]", i);
        }
        else if (i <= 48)
        {
            INFO_LOG(logger1, "hello world, I'm main, i[{}]", i);
        }
        else if (i <= 64)
        {
            WARN_LOG(logger1, "hello world, I'm main, i[{}]", i);
        }
        else if (i < 80)
        {
            ERROR_LOG(logger1, "hello world, I'm main, i[{}]", i);
        }
        else
        {
            FATAL_LOG(logger1, "hello world, I'm main, i[{}]", i);
        }
    }
    auto logger2 = logger::LoggerManager::getLogger("ceshi", "main_");
    logger2.setLevel(logger::LEVEL_INFO);
    for (int i = 0; i < 100; ++i)
    {
        INFO_LOG(logger2, "hello world, I'm main, i[{}]", i);
        if (30 == i)
        {
            logger2.setLevel(logger::LEVEL_WARN);
        }
        else if (80 == i)
        {
            logger2.setLevel(logger::LEVEL_DEBUG);
        }
    }
    auto logger3 = logger::LoggerManager::getLogger("test", "hw_");
    for (int i = 0; i < 100; ++i)
    {
        INFO_LOG(logger3, "hello world, I'm hw, i[{}]", i);
    }
    g_logger = logger::LoggerManager::getLogger();
    for (int i = 0; i < 100; ++i)
    {
        INFO_LOG(g_logger, "hello world, I'm default, i[{}]", i);
    }
}
