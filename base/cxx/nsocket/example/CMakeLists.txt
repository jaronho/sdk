cmake_minimum_required(VERSION 3.18.0)

project(example_nsocket)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
##########################################################################################

################################### 添加nsocket库 ###################################
option(enable_nsocket_openssl "是否启用套接字OpenSSL依赖" ON)

if(enable_nsocket_openssl)
    add_definitions(-DENABLE_NSOCKET_OPENSSL=1)
    find_package(OpenSSL 1.1.1 REQUIRED)
    if (OpenSSL_FOUND)
        message(STATUS "OpenSSL include path: ${OPENSSL_INCLUDE_DIR}")
        message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
        include_directories(${OPENSSL_INCLUDE_DIR})
    else()
        message(WARNING "OpenSSL not found")
        return()
    endif()
else()
    add_definitions(-DENABLE_NSOCKET_OPENSSL=0)
endif()

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

set(base_nsocket_path ..) # 设置nsocket库路径(可修改)
include_directories(${base_nsocket_path})

set(base_nsocket_core_src_list)
get_cxx_files(${base_nsocket_path}/nsocket/core src_list)
list(APPEND base_nsocket_core_src_list ${src_list})
source_group(3rdparty/base/nsocket/core FILES ${base_nsocket_core_src_list})

set(base_nsocket_tcp_src_list)
get_cxx_files(${base_nsocket_path}/nsocket/tcp src_list)
list(APPEND base_nsocket_tcp_src_list ${src_list})
source_group(3rdparty/base/nsocket/tcp FILES ${base_nsocket_tcp_src_list})

set(base_nsocket_src_list)
get_cxx_files(${base_nsocket_path}/nsocket src_list)
list(APPEND base_nsocket_src_list ${src_list})
source_group(3rdparty/base/nsocket FILES ${base_nsocket_src_list})

set(base_nsocket_files) # 工程中需要把nsocket的文件添加进去
list(APPEND base_nsocket_files ${base_nsocket_core_src_list}
                               ${base_nsocket_tcp_src_list}
                               ${base_nsocket_src_list})

print_info(BODY "base nsocket files:")
foreach(filename ${base_nsocket_files})
    message("    " ${filename})
endforeach()
##########################################################################################

# 添加客户端示例文件
set(example_client_files)
get_cxx_files(./client src_list)
list(APPEND example_client_files ${src_list})

print_info(BODY "example client files:")
foreach(filename ${example_client_files})
    message("    " ${filename})
endforeach()

# 添加服务端示例文件
set(example_server_files)
get_cxx_files(./server src_list)
list(APPEND example_server_files ${src_list})

print_info(BODY "example server files:")
foreach(filename ${example_server_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_nsocket_tcpclient ${base_nsocket_files} ${example_client_files})
add_executable(example_nsocket_tcpserver ${base_nsocket_files} ${example_server_files})

# 链接依赖库
target_link_libraries(example_nsocket_tcpclient Threads::Threads ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
target_link_libraries(example_nsocket_tcpserver Threads::Threads ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
