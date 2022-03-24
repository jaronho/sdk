cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14)

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[开关]
option(enable_database_crypto "是否启用数据库加解密" OFF)

[命令行]
find_package(OpenSSL 1.1.1 REQUIRED) # 如果启用数据库加密则需要添加
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

[链接库]
${OPENSSL_LIBRARIES}
]]

#########################################[ 输出 ]#########################################
#[[
[源目录]
${sqlite_include_dir}

[源目录]
${base_database_dir}

[源文件]
${base_database_files}
]]

##################################### 添加database库 #####################################
if(enable_database_crypto)
    set(sqlite_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/sqlcipher-4.4.3 CACHE INTERNAL "")
else()
    set(sqlite_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-3.36.0 CACHE INTERNAL "")
endif()

set(base_database_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(sqlite_src_list)
get_cxx_files(${sqlite_include_dir} src_list)
list(APPEND sqlite_src_list ${src_list})

set(base_winq_src_list)
get_cxx_files(winq src_list)
list(APPEND base_winq_src_list ${src_list})

set(base_database_src_list)
get_cxx_files(database src_list)
list(APPEND base_database_src_list ${src_list})

# 组装文件列表
set(base_database_files
    ${sqlite_src_list}
    ${base_winq_src_list}
    ${base_database_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base database files:")
foreach(filename ${base_database_files})
    message("    " ${filename})
endforeach()
