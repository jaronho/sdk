#include "logger/logger_manager.h"

#include <iostream>

int main()
{
    logger::LoggerManager::start("log", "app", "who");
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
    logger2.setLevel(logger::Level::INFO);
    for (int i = 0; i < 100; ++i)
    {
        INFO_LOG(logger2, "hello world, I'm main, i[{}]", i);
        if (30 == i)
        {
            logger2.setLevel(logger::Level::WARN);
        }
        else if (80 == i)
        {
            logger2.setLevel(logger::Level::DEBUG);
        }
    }
    auto logger3 = logger::LoggerManager::getLogger("test", "hw_");
    for (int i = 0; i < 100; ++i)
    {
        INFO_LOG(logger3, "hello world, I'm hw, i[{}]", i);
    }
}
