cmake_minimum_required(VERSION 3.18.0)

project(example_rpc)

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

# 添加rpc公共库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/comlib/rpc)
include_directories(${comlib_rpc_dir})
source_group(TREE ${comlib_rpc_dir} PREFIX 3rdparty/comlib FILES ${comlib_rpc_broker_files})
source_group(TREE ${comlib_rpc_dir} PREFIX 3rdparty/comlib FILES ${comlib_rpc_client_files})

# 构建可执行程序
add_executable(broker ${base_nsocket_files} ${base_threading_files} ${base_utility_files} ${comlib_rpc_broker_files} broker.cpp)
add_executable(example_client ${base_algorithm_files} ${base_nsocket_files} ${base_threading_files} ${base_utility_files} ${comlib_rpc_client_files} example_client.cpp)
add_executable(demo_client1 ${base_algorithm_files} ${base_nsocket_files} ${base_threading_files} ${base_utility_files} ${comlib_rpc_client_files} demo_client1.cpp demo_def.h)
add_executable(demo_client2 ${base_algorithm_files} ${base_nsocket_files} ${base_threading_files} ${base_utility_files} ${comlib_rpc_client_files} demo_client2.cpp demo_def.h)
add_executable(demo_client3 ${base_algorithm_files} ${base_nsocket_files} ${base_threading_files} ${base_utility_files} ${comlib_rpc_client_files} demo_client3.cpp demo_def.h)

# 链接依赖库
if(enable_nsocket_openssl)
    target_link_libraries(broker Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(example_client Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(demo_client1 Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(demo_client2 Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
    target_link_libraries(demo_client3 Threads::Threads ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES})
else()
    target_link_libraries(broker Threads::Threads ${Boost_LIBRARIES})
    target_link_libraries(example_client Threads::Threads ${Boost_LIBRARIES})
    target_link_libraries(demo_client1 Threads::Threads ${Boost_LIBRARIES})
    target_link_libraries(demo_client2 Threads::Threads ${Boost_LIBRARIES})
    target_link_libraries(demo_client3 Threads::Threads ${Boost_LIBRARIES})
endif()
