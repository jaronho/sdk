cmake_minimum_required(VERSION 3.18.0)

project(example_httpclient)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

####################################### 添加CURL库 #######################################
option(use_curl_staticlib "是否使用CURL静态库" OFF) # ON表示链接CURL静态库, OFF或者不设置表示链接动态库(目前只支持Windows)
find_package(CURL 8.4.0 REQUIRED)
if(CURL_FOUND) # 注意: curl库区分ssl和非ssl
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        if(use_curl_staticlib)
            add_definitions("-DCURL_STATICLIB") # 如果链接静态库则必须添加这个宏
        endif()
        set(CURL_INCLUDE_DIRS $ENV{CURL_HOME}/include) # 读取环境变量`CURL_HOME`的值
    endif()
    set(CURL_LIBRARY_DIRS ${CURL_INCLUDE_DIRS}/../lib)
    message(STATUS "CURL include path: ${CURL_INCLUDE_DIRS}")
    message(STATUS "CURL library path: ${CURL_LIBRARY_DIRS}")
    include_directories(${CURL_INCLUDE_DIRS})
    link_directories(${CURL_LIBRARY_DIRS})
else()
    message(WARNING "CURL not found")
    return()
endif()
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
##########################################################################################

# 添加curlex基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/curlex 3rdparty/base/curlex)
include_directories(${base_curlex_dir})
source_group(TREE ${base_curlex_dir} PREFIX 3rdparty/base FILES ${base_curlex_files})

# 添加threading基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/threading 3rdparty/base/threading)
include_directories(${base_threading_dir})
source_group(TREE ${base_threading_dir} PREFIX 3rdparty/base FILES ${base_threading_files})

# 添加httpclient公共库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/comlib/httpclient)
include_directories(${comlib_httpclient_dir})
source_group(TREE ${comlib_httpclient_dir} PREFIX 3rdparty/comlib FILES ${comlib_httpclient_files})

# 添加示例文件
set(example_files)
get_cxx_files(${CMAKE_CURRENT_SOURCE_DIR} src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_httpclient ${base_curlex_files} ${base_threading_files} ${comlib_httpclient_files} ${example_files})

# 链接依赖库
target_link_libraries(example_httpclient Threads::Threads ${curl_libraries} ${Boost_LIBRARIES})

# 拷贝动态链接库
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_custom_command(
        TARGET example_httpclient
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CURL_INCLUDE_DIRS}/../bin/libcurl.dll "$(OutDir)/libcurl.dll"
        COMMAND ${CMAKE_COMMAND} -E copy ${CURL_INCLUDE_DIRS}/../bin/libcurl-d.dll "$(OutDir)/libcurl-d.dll"
    )
endif()
