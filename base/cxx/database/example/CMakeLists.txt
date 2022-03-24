cmake_minimum_required(VERSION 3.18.0)

project(example_database)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

###################################### 添加OPENSSL库 #####################################
find_package(OpenSSL 1.1.1 REQUIRED) # 如果启用数据库加密则需要添加以下
if (OpenSSL_FOUND)
    message(STATUS "OpenSSL include path: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
    add_definitions(-DSQLITE_HAS_CODEC)
    add_definitions(-DSQLCIPHER_CRYPTO_OPENSSL)
    add_definitions(-DSQLITE_TEMP_STORE=2)
    include_directories(${OPENSSL_INCLUDE_DIR})
else()
    message(WARNING "OpenSSL not found")
    return()
endif()
##########################################################################################

# 添加database基础库
option(enable_database_crypto "是否启用数据库加解密" OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/database)
include_directories(${sqlite_include_dir})
include_directories(${base_database_dir})
source_group(TREE ${base_database_dir} PREFIX 3rdparty/base FILES ${base_database_files})

# 添加示例文件
set(example_files)
get_cxx_files(${CMAKE_CURRENT_SOURCE_DIR} src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_database ${base_database_files} ${example_files})

# 链接依赖库
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(enable_database_crypto)
        target_link_libraries(example_database dl Threads::Threads ${OPENSSL_LIBRARIES})
    else()
        target_link_libraries(example_database dl Threads::Threads)
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    if(enable_database_crypto)
        target_link_libraries(example_database ${OPENSSL_LIBRARIES})
    endif()
else()
    message(WARNING "Other Platform: ${CMAKE_SYSTEM_NAME}")
endif()
