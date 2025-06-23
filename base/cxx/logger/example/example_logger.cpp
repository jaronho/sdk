#include <iostream>

#include "logger/logger_manager.h"

logger::Logger g_logger;

int main()
{
    logger::LogConfig cfg;
    cfg.path = "log";
    cfg.name = "app"; /* 默认的日志记录器名称 */
    cfg.fileExtName = ".log";
    cfg.level = logger::LEVEL_TRACE;
#if 0
    /* 设置不同的日志等级记录到指定的文件中 */
    cfg.levelFile = {{logger::LEVEL_TRACE, logger::LEVEL_TRACE}, {logger::LEVEL_DEBUG, logger::LEVEL_DEBUG},
                     {logger::LEVEL_INFO, logger::LEVEL_INFO},   {logger::LEVEL_WARN, logger::LEVEL_WARN},
                     {logger::LEVEL_ERROR, logger::LEVEL_ERROR},  {logger::LEVEL_FATAL, logger::LEVEL_FATAL}};
#endif
    cfg.fileMaxSize = 20 * 1024 * 1024;
    cfg.fileMaxCount = 5;
    cfg.fileIndexFixed = true;
    cfg.newFolderDaily = true;
    logger::LoggerManager::setConfig(cfg, "who");
    auto logger1 = logger::LoggerManager::getLogger("test", -1, "main_");
    logger1.setConsoleMode(2);
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
    auto logger2 = logger::LoggerManager::getLogger("ceshi", -1, "main_");
    logger2.setConsoleMode(2);
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
    auto logger3 = logger::LoggerManager::getLogger("test", -1, "hw_");
    for (int i = 0; i < 100; ++i)
    {
        INFO_LOG(logger3, "hello world, I'm hw, i[{}]", i);
    }
    g_logger = logger::LoggerManager::getLogger();
    g_logger.setConsoleMode(2);
    for (int i = 0; i < 100; ++i)
    {
        INFO_LOG(g_logger, "hello world, I'm default, i[{}]", i);
    }
    int numInt = 12;
    float numFloat = 345.152723f;
    INFO_LOG(g_logger, "numInt === {} === {:04d} === {:x} === {:X}", numInt, numInt, numInt, numInt);
    INFO_LOG(g_logger, "numFloat === {} === {:.1f} === {:.2f} === {:.3f}", numFloat, numFloat, numFloat, numFloat);
}
