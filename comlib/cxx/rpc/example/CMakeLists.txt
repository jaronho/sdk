cmake_minimum_required(VERSION 3.18.0)

project(example_rpc)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
##########################################################################################

################################### 添加algorithm库 ###################################
set(base_algorithm_path ../../../../base/cxx/algorithm) # 设置algorithm库路径(可修改)
include_directories(${base_algorithm_path})

set(base_algorithm_snowflake_src_list)
get_cxx_files(${base_algorithm_path}/algorithm/snowflake src_list)
list(APPEND base_algorithm_snowflake_src_list ${src_list})
source_group(3rdparty/base/algorithm/snowflake FILES ${base_algorithm_snowflake_src_list})

set(base_algorithm_files) # 工程中需要把algorithm的文件添加进去
list(APPEND base_algorithm_files ${base_algorithm_snowflake_src_list})

print_info(BODY "base algorithm files:")
foreach(filename ${base_algorithm_files})
    message("    " ${filename})
endforeach()
##########################################################################################

################################### 添加nsocket库 ###################################
option(enable_nsocket_openssl "是否启用套接字OpenSSL依赖" ON)

set(Boost_USE_STATIC_LIBS ON) # ON表示链接Boost静态库, OFF或者不设置表示链接动态库
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

set(base_nsocket_path ../../../../base/cxx/nsocket) # 设置nsocket库路径(可修改)
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

################################### 添加utilitiy库 ###################################
set(base_utilitiy_path ../../../../base/cxx/utilitiy) # 设置utilitiy库路径(可修改)
include_directories(${base_utilitiy_path})

set(base_utilitiy_bytearray_src_list)
get_cxx_files(${base_utilitiy_path}/utilitiy/bytearray src_list)
list(APPEND base_utilitiy_bytearray_src_list ${src_list})
source_group(3rdparty/base/utilitiy/bytearray FILES ${base_utilitiy_bytearray_src_list})

set(base_utilitiy_files) # 工程中需要把utilitiy的文件添加进去
list(APPEND base_utilitiy_files ${base_utilitiy_bytearray_src_list})

print_info(BODY "base utilitiy files:")
foreach(filename ${base_utilitiy_files})
    message("    " ${filename})
endforeach()
##########################################################################################

# 添加代理服务文件
set(broker_files)
list(APPEND broker_files ../rpc/rpc_broker.cpp
                         ../rpc/rpc_broker.h
                         ../rpc/rpc_msg.hpp
                         broker.cpp)

print_info(BODY "broker files:")
foreach(filename ${broker_files})
    message("    " ${filename})
endforeach()

# 添加客户端示例文件
set(example_client_files)
list(APPEND example_client_files ../rpc/rpc_client.cpp
                                 ../rpc/rpc_client.h
                                 ../rpc/rpc_msg.hpp
                                 example_client.cpp)

print_info(BODY "example client files:")
foreach(filename ${example_client_files})
    message("    " ${filename})
endforeach()

# 添加demo客户端1文件
set(demo_client1_files)
list(APPEND demo_client1_files ../rpc/rpc_client.cpp
                               ../rpc/rpc_client.h
                               ../rpc/rpc_msg.hpp
                               demo_client1.cpp
                               demo_def.h)

print_info(BODY "demo client1 files:")
foreach(filename ${demo_client1_files})
    message("    " ${filename})
endforeach()

# 添加demo客户端2文件
set(demo_client2_files)
list(APPEND demo_client2_files ../rpc/rpc_client.cpp
                               ../rpc/rpc_client.h
                               ../rpc/rpc_msg.hpp
                               demo_client2.cpp
                               demo_def.h)

print_info(BODY "demo client2 files:")
foreach(filename ${demo_client2_files})
    message("    " ${filename})
endforeach()

# 添加demo客户端3文件
set(demo_client3_files)
list(APPEND demo_client3_files ../rpc/rpc_client.cpp
                               ../rpc/rpc_client.h
                               ../rpc/rpc_msg.hpp
                               demo_client3.cpp
                               demo_def.h)

print_info(BODY "demo client3 files:")
foreach(filename ${demo_client3_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(broker ${base_nsocket_files} ${base_utilitiy_files} ${broker_files})
add_executable(example_client ${base_algorithm_files} ${base_nsocket_files} ${base_utilitiy_files} ${example_client_files})
add_executable(demo_client1 ${base_algorithm_files} ${base_nsocket_files} ${base_utilitiy_files} ${demo_client1_files})
add_executable(demo_client2 ${base_algorithm_files} ${base_nsocket_files} ${base_utilitiy_files} ${demo_client2_files})
add_executable(demo_client3 ${base_algorithm_files} ${base_nsocket_files} ${base_utilitiy_files} ${demo_client3_files})

# 链接依赖库
target_link_libraries(broker Threads::Threads ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
target_link_libraries(example_client Threads::Threads ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
target_link_libraries(demo_client1 Threads::Threads ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
target_link_libraries(demo_client2 Threads::Threads ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
target_link_libraries(demo_client3 Threads::Threads ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
