# CMake版本
cmake_minimum_required(VERSION 3.18.0)

# `std::make_unique`要求最低C++14
set(CMAKE_CXX_STANDARD 14)

# 加载自定义模块
include(cmake/tools.cmake)

# 工程名
project(widgets)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

####################################### 添加CURL库 #######################################
option(use_curl_staticlib "是否使用CURL静态库" OFF) # ON表示链接CURL静态库, OFF或者不设置表示链接动态库(目前只支持Windows)
find_package(CURL 7.76.1 REQUIRED)
if (CURL_FOUND) # 注意: curl库区分ssl和非ssl
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        if (use_curl_staticlib)
            add_definitions("-DCURL_STATICLIB") # 如果链接静态库则必须添加这个宏
        endif ()
        set(CURL_INCLUDE_DIRS $ENV{CURL_HOME}/include) # 读取环境变量`CURL_HOME`的值
    endif ()
    set(CURL_LIBRARY_DIRS ${CURL_INCLUDE_DIRS}/../lib)
    message(STATUS "CURL include path: ${CURL_INCLUDE_DIRS}")
    message(STATUS "CURL library path: ${CURL_LIBRARY_DIRS}")
    include_directories(${CURL_INCLUDE_DIRS})
    link_directories(${CURL_LIBRARY_DIRS})
else ()
    message(WARNING "CURL not found")
    return()
endif ()
##########################################################################################

###################################### 添加OPENSSL库 #####################################
find_package(OpenSSL 1.1.1 REQUIRED) # 如果启用套接字加密则需要添加
if (OpenSSL_FOUND)
    message(STATUS "OpenSSL include path: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
    include_directories(${OPENSSL_INCLUDE_DIR})
else ()
    message(WARNING "OpenSSL not found")
    return()
endif ()
##########################################################################################

####################################### 添加BOOST库 ######################################
set(Boost_USE_STATIC_LIBS ON) # ON表示链接Boost静态库, OFF或者不设置表示链接动态库
option(enable_fiber "是否启用Boost库的Fiber模块" OFF)
if (enable_fiber)
    find_package(Boost 1.76.0 REQUIRED COMPONENTS date_time fiber)
    add_definitions(-DENABLE_THREADING_FIBER=1)
else ()
    find_package(Boost 1.76.0 REQUIRED COMPONENTS date_time)
    add_definitions(-DENABLE_THREADING_FIBER=0)
endif ()
if (Boost_FOUND)
    message(STATUS "Boost include path: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost library path: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else ()
    message(WARNING "Boost not found")
    return()
endif ()
##########################################################################################

####################################### 添加pcap库 #######################################
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_path(PCAP_INCLUDE_DIR NAMES pcap.h)
    find_library(PCAP_LIBRARIES NAMES wpcap HINTS ${PCAP_INCLUDE_DIR}/../Lib/x64) # 如果工程要生成32位的, 则不需要加x64
else ()
    if ("$ENV{PCAP_ROOT}" STREQUAL "") # 自动查找
        find_path(PCAP_INCLUDE_DIR NAMES pcap.h)
    else () # 如果有手动配置PCAP环境变量PCAP_ROOT, 则从环境变量中获取
        set(PCAP_INCLUDE_DIR $ENV{PCAP_ROOT}/include)
    endif ()
    find_library(PCAP_LIBRARIES NAMES pcap HINTS ${PCAP_INCLUDE_DIR}/../lib)
endif ()
set(PCAP_ROOT_DIR ${PCAP_INCLUDE_DIR}/../)
message(STATUS "Pcap root dir: ${PCAP_ROOT_DIR}")
message(STATUS "Pcap include path: ${PCAP_INCLUDE_DIR}")
message(STATUS "Pcap libraries: ${PCAP_LIBRARIES}")
if ("${PCAP_ROOT_DIR}" STREQUAL "PCAP_ROOT_DIR-NOTFOUND" OR
        "${PCAP_INCLUDE_DIR}" STREQUAL "PCAP_INCLUDE_DIR-NOTFOUND" OR
        "${PCAP_LIBRARIES}" STREQUAL "PCAP_LIBRARIES-NOTFOUND")
    message(WARNING "Pcap not found")
    return()
else ()
    include_directories(${PCAP_INCLUDE_DIR})
endif ()
##########################################################################################

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    # 设置版本类型Debug/Release
    set(outputDir "debug")
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
        set(CMAKE_BUILD_TYPE "Debug")
    endif ()
    if (${CMAKE_BUILD_TYPE} MATCHES "Release")
        message(STATUS "Release版本")
        set(BuildType "Release")
        set(outputDir "release")
    else ()
        message(STATUS "Debug版本")
        set(BuildType "Debug")
    endif ()
endif ()

# 添加应用目录
add_subdirectory(proc_tcpserver)
