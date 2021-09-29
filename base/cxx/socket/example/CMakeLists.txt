cmake_minimum_required(VERSION 3.18.0)

project(example_socket)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
##########################################################################################

################################### 添加socket库 ###################################
find_package(Boost 1.76.0 REQUIRED)

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

set(base_socket_path ..) # 设置socket库路径(可修改)
include_directories(${base_socket_path})

set(base_socket_core_src_list)
get_cxx_files(${base_socket_path}/socket/core src_list)
list(APPEND base_socket_core_src_list ${src_list})
source_group(3rdparty/base/socket/core FILES ${base_socket_core_src_list})

set(base_socket_tcp_src_list)
get_cxx_files(${base_socket_path}/socket/tcp src_list)
list(APPEND base_socket_tcp_src_list ${src_list})
source_group(3rdparty/base/socket/tcp FILES ${base_socket_tcp_src_list})

set(base_threading_src_list)
get_cxx_files(${base_socket_path}/socket src_list)
list(APPEND base_threading_src_list ${src_list})
source_group(3rdparty/base/socket FILES ${base_threading_src_list})

set(base_socket_files) # 工程中需要把socket的文件添加进去
list(APPEND base_socket_files ${base_socket_core_src_list}
                              ${base_socket_tcp_src_list}
                              ${base_threading_src_list})

print_info(BODY "base socket files:")
foreach(filename ${base_socket_files})
    message("    " ${filename})
endforeach()
##########################################################################################

# 添加示例文件
set(example_files)
get_cxx_files(. src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_socket_tcpclient ${base_socket_files} ${example_files})
add_executable(example_socket_tcpserver ${base_socket_files} ${example_files})

# 链接依赖库
target_link_libraries(example_threading Threads::Threads ${Boost_LIBRARIES})
