cmake_minimum_required(VERSION 3.18.0)

project(example_nsocket)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

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

####################################### 添加BOOST库 ######################################
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
##########################################################################################

# 添加nsocket基础库
option(enable_nsocket_openssl "是否启用套接字OpenSSL依赖" ON)
if(enable_nsocket_openssl)
    add_definitions(-DENABLE_NSOCKET_OPENSSL=1)
else()
    add_definitions(-DENABLE_NSOCKET_OPENSSL=0)
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/nsocket)
include_directories(${base_nsocket_dir})
source_group(TREE ${base_nsocket_dir} PREFIX 3rdparty/base FILES ${base_nsocket_files})

# 添加TCP客户端示例文件
set(example_tcpclient_files)
list(APPEND example_tcpclient_files client/example_socket_tcpclient.cpp)

print_info(BODY "example tcp client files:")
foreach(filename ${example_tcpclient_files})
    message("    " ${filename})
endforeach()

# 添加WebSocket客户端示例文件
set(example_websocketclient_files)
list(APPEND example_websocketclient_files client/example_socket_websocketclient.cpp)

print_info(BODY "example websocket client files:")
foreach(filename ${example_websocketclient_files})
    message("    " ${filename})
endforeach()

# 添加TCP服务端示例文件
set(example_tcpserver_files)
list(APPEND example_tcpserver_files server/example_socket_tcpserver.cpp)

print_info(BODY "example tcp server files:")
foreach(filename ${example_tcpserver_files})
    message("    " ${filename})
endforeach()

# 添加HTTP服务端示例文件
set(example_httpserver_files)
list(APPEND example_httpserver_files server/example_socket_httpserver.cpp)

print_info(BODY "example http server files:")
foreach(filename ${example_httpserver_files})
    message("    " ${filename})
endforeach()

# 添加WebSocket服务端示例文件
set(example_websocketserver_files)
list(APPEND example_websocketserver_files server/example_socket_websocketserver.cpp)

print_info(BODY "example websocket server files:")
foreach(filename ${example_websocketserver_files})
    message("    " ${filename})
endforeach()

# 添加FTP服务端示例文件
set(example_ftpserver_files)
list(APPEND example_ftpserver_files server/example_socket_ftpserver.cpp)

print_info(BODY "example ftp server files:")
foreach(filename ${example_ftpserver_files})
    message("    " ${filename})
endforeach()

if (MSVC)
    add_compile_options("/utf-8") # 添加UTF8编码支持
endif()

# 构建可执行程序
add_executable(example_nsocket_tcpclient ${base_nsocket_tcp_files} ${example_tcpclient_files})
add_executable(example_nsocket_websocketclient ${base_nsocket_websocket_files} ${example_websocketclient_files})
add_executable(example_nsocket_tcpserver ${base_nsocket_tcp_files} ${example_tcpserver_files})
add_executable(example_nsocket_httpserver ${base_nsocket_http_files} ${example_httpserver_files})
add_executable(example_nsocket_websocketserver ${base_nsocket_websocket_files} ${example_websocketserver_files})
add_executable(example_nsocket_ftpserver ${base_nsocket_ftp_files} ${example_ftpserver_files})

# 链接依赖库
if(enable_nsocket_openssl)
    target_link_libraries(example_nsocket_tcpclient Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(example_nsocket_websocketclient Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(example_nsocket_tcpserver Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(example_nsocket_httpserver Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(example_nsocket_websocketserver Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(example_nsocket_ftpserver Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
else()
    target_link_libraries(example_nsocket_tcpclient Threads::Threads ${Boost_LIBRARIES})
    target_link_libraries(example_nsocket_websocketclient Threads::Threads ${Boost_LIBRARIES})
    target_link_libraries(example_nsocket_tcpserver Threads::Threads ${Boost_LIBRARIES})
    target_link_libraries(example_nsocket_httpserver Threads::Threads ${Boost_LIBRARIES})
    target_link_libraries(example_nsocket_websocketserver Threads::Threads ${Boost_LIBRARIES})
    target_link_libraries(example_nsocket_ftpserver Threads::Threads ${Boost_LIBRARIES})
endif()
