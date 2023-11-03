cmake_minimum_required(VERSION 3.18.0)

project(example_threading)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

####################################### 添加BOOST库 ######################################
set(Boost_USE_STATIC_LIBS ON) # ON表示链接Boost静态库, OFF或者不设置表示链接动态库
option(enable_fiber "是否启用Boost库的Fiber模块" ON)
if(enable_fiber)
    find_package(Boost 1.76.0 REQUIRED COMPONENTS date_time fiber)
    add_definitions(-DENABLE_THREADING_FIBER=1)
else()
    find_package(Boost 1.76.0 REQUIRED COMPONENTS date_time)
    add_definitions(-DENABLE_THREADING_FIBER=0)
endif()
if(Boost_FOUND)
    message(STATUS "Boost include path: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost library path: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(WARNING "Boost not found")
    return()
endif()
if (CMAKE_SYSTEM_NAME MATCHES "Windows") # 增加`WIN32_LEAN_AND_MEAN`宏定义避免平台头文件重复引用导致错误
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()
##########################################################################################

# 添加threading基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/threading)
include_directories(${base_threading_dir})
source_group(TREE ${base_threading_dir} PREFIX 3rdparty/base FILES ${base_threading_files})

# 添加asio示例文件
set(example_asio_files)
list(APPEND example_asio_files example_asio_executor.cpp)

print_info(BODY "example asio files:")
foreach(filename ${example_asio_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_asio ${base_threading_files} ${example_asio_files})

# 链接依赖库
target_link_libraries(example_asio Threads::Threads ${Boost_LIBRARIES})
