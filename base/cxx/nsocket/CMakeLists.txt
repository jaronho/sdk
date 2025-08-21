cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[开关]
option(enable_nsocket_openssl "是否启用套接字OpenSSL依赖" ON)
if(enable_nsocket_openssl)
    add_definitions(-DENABLE_NSOCKET_OPENSSL=1)
else()
    add_definitions(-DENABLE_NSOCKET_OPENSSL=0)
endif()

[命令行]
find_package(Threads REQUIRED)

[命令行]
find_package(OpenSSL 1.1.1 REQUIRED) # 如果启用套接字加密则需要添加
if (OpenSSL_FOUND)
    message(STATUS "OpenSSL include path: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
    include_directories(${OPENSSL_INCLUDE_DIR})
else()
    message(WARNING "OpenSSL not found")
    return()
endif()

[命令行]
set(Boost_USE_STATIC_LIBS ON) # ON表示链接Boost静态库, OFF或者不设置表示链接动态库
find_package(Boost 1.76.0 REQUIRED COMPONENTS date_time)
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

[链接库]
Threads::Threads

[链接库]
${OPENSSL_LIBRARIES}

[链接库]
${Boost_LIBRARIES}
]]

#########################################[ 输出 ]#########################################
#[[
[源目录]
${base_nsocket_dir}

[源文件]
${base_nsocket_files}

[源文件]
${base_nsocket_tcp_files}

[源文件]
${base_nsocket_udp_files}

[源文件]
${base_nsocket_http_files}

[源文件]
${base_nsocket_websocket_files}

[源文件]
${base_nsocket_ftp_files}
]]

###################################### 添加nsocket库 #####################################
set(base_nsocket_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(base_nsocket_core_src_list)
get_cxx_files(nsocket/core src_list)
list(APPEND base_nsocket_core_src_list ${src_list})

set(base_nsocket_http_src_list)
get_cxx_files(nsocket/http src_list)
list(APPEND base_nsocket_http_src_list ${src_list})

set(base_nsocket_tcp_src_list)
get_cxx_files(nsocket/tcp src_list)
list(APPEND base_nsocket_tcp_src_list ${src_list})

set(base_nsocket_udp_src_list)
get_cxx_files(nsocket/udp src_list)
list(APPEND base_nsocket_udp_src_list ${src_list})

set(base_nsocket_websocket_base64_src_list)
get_cxx_files(nsocket/websocket/base64 src_list)
list(APPEND base_nsocket_websocket_base64_src_list ${src_list})

set(base_nsocket_websocket_sha1_src_list)
get_cxx_files(nsocket/websocket/sha1 src_list)
list(APPEND base_nsocket_websocket_sha1_src_list ${src_list})

set(base_nsocket_websocket_src_list)
get_cxx_files(nsocket/websocket src_list)
list(APPEND base_nsocket_websocket_src_list ${src_list})

set(base_nsocket_ftp_src_list)
get_cxx_files(nsocket/ftp src_list)
list(APPEND base_nsocket_ftp_src_list ${src_list})

set(base_nsocket_src_list)
get_cxx_files(nsocket src_list)
list(APPEND base_nsocket_src_list ${src_list})

# 组装文件列表
set(base_nsocket_files
    ${base_nsocket_core_src_list}
    ${base_nsocket_http_src_list}
    ${base_nsocket_tcp_src_list}
    ${base_nsocket_udp_src_list}
    ${base_nsocket_websocket_base64_src_list}
    ${base_nsocket_websocket_sha1_src_list}
    ${base_nsocket_websocket_src_list}
    ${base_nsocket_ftp_src_list}
    ${base_nsocket_src_list}
    CACHE INTERNAL "")

set(base_nsocket_tcp_files
    ${base_nsocket_core_src_list}
    ${base_nsocket_tcp_src_list}
    ${base_nsocket_src_list}
    CACHE INTERNAL "")

set(base_nsocket_udp_files
    ${base_nsocket_core_src_list}
    ${base_nsocket_udp_src_list}
    ${base_nsocket_src_list}
    CACHE INTERNAL "")

set(base_nsocket_http_files
    ${base_nsocket_core_src_list}
    ${base_nsocket_http_src_list}
    ${base_nsocket_tcp_src_list}
    ${base_nsocket_src_list}
    CACHE INTERNAL "")

set(base_nsocket_websocket_files
    ${base_nsocket_core_src_list}
    ${base_nsocket_tcp_src_list}
    ${base_nsocket_websocket_base64_src_list}
    ${base_nsocket_websocket_sha1_src_list}
    ${base_nsocket_websocket_src_list}
    ${base_nsocket_src_list}
    CACHE INTERNAL "")

set(base_nsocket_ftp_files
    ${base_nsocket_core_src_list}
    ${base_nsocket_tcp_src_list}
    ${base_nsocket_ftp_src_list}
    ${base_nsocket_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base nsocket files:")
foreach(filename ${base_nsocket_files})
    message("    " ${filename})
endforeach()
