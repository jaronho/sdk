cmake_minimum_required(VERSION 3.18.0)

project(example_serial)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

# 添加serial基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/serial)
include_directories(${base_serial_dir})
source_group(TREE ${base_serial_dir} PREFIX 3rdparty/base FILES ${base_serial_files})

if (MSVC)
    add_compile_options("/utf-8") # 添加UTF8编码支持
endif()

# 构建可执行程序
add_executable(serial_tool ${base_serial_files} serial_tool.cpp)

# 链接依赖库文件
target_link_libraries(serial_tool Threads::Threads)
