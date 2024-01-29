cmake_minimum_required(VERSION 3.18.0)

project(example_curlex)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

###################################### 添加OPENSSL库 #####################################
find_package(OpenSSL 1.1.1 REQUIRED) # 如果启用套接字加密则需要添加
if (OpenSSL_FOUND)
    message(STATUS "OpenSSL include path: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
    include_directories(${OPENSSL_INCLUDE_DIR})
else()
    message(WARNING "OpenSSL not found")
    return()
endif()
##########################################################################################

####################################### 添加CURL库 #######################################
option(use_curl_staticlib "是否使用CURL静态库" OFF) # ON表示链接CURL静态库, OFF或者不设置表示链接动态库(目前只支持Windows)
find_package(CURL 8.4.0 REQUIRED)
if(CURL_FOUND) # 注意: curl库区分ssl和非ssl
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        if(use_curl_staticlib)
            add_definitions(-DCURL_STATICLIB) # 如果链接静态库则必须添加这个宏
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

# 添加curlex基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/curlex)
include_directories(${base_curlex_dir})
source_group(TREE ${base_curlex_dir} PREFIX 3rdparty/base FILES ${base_curlex_files})

# 添加示例文件
set(example_files)
get_cxx_files(${CMAKE_CURRENT_SOURCE_DIR} src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

if (MSVC)
    add_compile_options("/utf-8") # 添加UTF8编码支持
endif()

# 构建可执行程序
add_executable(example_curlex ${base_curlex_files} ${example_files})

# 链接依赖库
target_link_libraries(example_curlex ${OPENSSL_LIBRARIES} ${curl_libraries})

# 拷贝动态链接库
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_custom_command(
        TARGET example_curlex
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${OPENSSL_INCLUDE_DIR}/../bin/libcrypto-1_1-x64.dll "$(OutDir)/libcrypto-1_1-x64.dll"
        COMMAND ${CMAKE_COMMAND} -E copy ${OPENSSL_INCLUDE_DIR}/../bin/libssl-1_1-x64.dll "$(OutDir)/libssl-1_1-x64.dll"
        COMMAND ${CMAKE_COMMAND} -E copy ${CURL_INCLUDE_DIRS}/../bin/libcurl.dll "$(OutDir)/libcurl.dll"
        COMMAND ${CMAKE_COMMAND} -E copy ${CURL_INCLUDE_DIRS}/../bin/libcurl-d.dll "$(OutDir)/libcurl-d.dll"
    )
endif()
