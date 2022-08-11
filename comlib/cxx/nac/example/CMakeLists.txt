cmake_minimum_required(VERSION 3.18.0)

project(example_nac)

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

# 添加algorithm基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/algorithm 3rdparty/base/algorithm)
include_directories(${base_algorithm_dir})
source_group(TREE ${base_algorithm_dir} PREFIX 3rdparty/base FILES ${base_algorithm_files})

# 添加fileparse基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/fileparse 3rdparty/base/fileparse)
include_directories(${base_fileparse_dir})
source_group(TREE ${base_fileparse_dir} PREFIX 3rdparty/base FILES ${base_fileparse_files})

# 添加logger基础库
option(enable_logger_detail "是否启用日志详情(包含文件名,行号,函数名)" ON)
if(enable_logger_detail)
    add_definitions(-DENABLE_LOGGER_DETAIL=2)
else()
    add_definitions(-DENABLE_LOGGER_DETAIL=0)
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/logger 3rdparty/base/logger)
include_directories(${fmt_include_dir})
include_directories(${base_logger_dir})
source_group(TREE ${base_logger_dir} PREFIX 3rdparty/base FILES ${base_logger_files})

# 添加nsocket基础库
option(enable_nsocket_openssl "是否启用套接字OpenSSL依赖" ON)
if(enable_nsocket_openssl)
    add_definitions(-DENABLE_NSOCKET_OPENSSL=1)
else()
    add_definitions(-DENABLE_NSOCKET_OPENSSL=0)
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/nsocket 3rdparty/base/nsocket)
include_directories(${base_nsocket_dir})
source_group(TREE ${base_nsocket_dir} PREFIX 3rdparty/base FILES ${base_nsocket_files})

# 添加threading基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/threading 3rdparty/base/threading)
include_directories(${base_threading_dir})
source_group(TREE ${base_threading_dir} PREFIX 3rdparty/base FILES ${base_threading_files})

# 添加utility基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/utility 3rdparty/base/utility)
include_directories(${base_utility_dir})
source_group(TREE ${base_utility_dir} PREFIX 3rdparty/base FILES ${base_utility_files})

# 添加nac公共库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/comlib/nac)
include_directories(${comlib_nac_dir})
source_group(TREE ${comlib_nac_dir} PREFIX 3rdparty/comlib FILES ${comlib_nac_files})

# 添加客户端示例文件
set(example_client_files)
list(APPEND example_client_files access_def.h example_client.cpp)

print_info(BODY "example client files:")
foreach(filename ${example_client_files})
    message("    " ${filename})
endforeach()

# 添加服务端示例文件
set(example_server_files)
list(APPEND example_server_files access_def.h example_server.cpp)

print_info(BODY "example server files:")
foreach(filename ${example_server_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_client
               ${base_algorithm_files}
               ${base_fileparse_files}
               ${base_logger_files}
               ${base_nsocket_files}
               ${base_threading_files}
               ${base_utility_files}
               ${comlib_nac_files}
               ${example_client_files})

add_executable(example_server
               ${base_algorithm_files}
               ${base_fileparse_files}
               ${base_logger_files}
               ${base_nsocket_files}
               ${base_threading_files}
               ${base_utility_files}
               ${example_server_files})

# 链接依赖库
if(enable_nsocket_openssl)
    target_link_libraries(example_client Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(example_server Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
else()
    target_link_libraries(example_client Threads::Threads ${Boost_LIBRARIES})
    target_link_libraries(example_server Threads::Threads ${Boost_LIBRARIES})
endif()
