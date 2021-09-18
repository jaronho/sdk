cmake_minimum_required(VERSION 3.18.0)

project(example_database)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加database库 ###################################
option(enable_database_crypto "是否启用数据库加解密" ON)

set(base_database_path ..) # 设置database库路径(可修改)
include_directories(${base_database_path})

if(enable_database_crypto)
    include_directories(${base_database_path}/sqlcipher-4.4.3)
    
    add_definitions(-DSQLITE_HAS_CODEC)
    add_definitions(-DSQLCIPHER_CRYPTO_OPENSSL)
    add_definitions(-DSQLITE_TEMP_STORE=2)
    
    set(sqlcipher_src_list)
    get_cxx_files(${base_database_path}/sqlcipher-4.4.3 src_list)
    list(APPEND sqlcipher_src_list ${src_list})
    source_group(3rdparty/base/sqlcipher-4.4.3 FILES ${sqlcipher_src_list})
else()
    include_directories(${base_database_path}/sqlite-3.36.0)
    
    set(sqlite_src_list)
    get_cxx_files(${base_database_path}/sqlite-3.36.0 src_list)
    list(APPEND sqlite_src_list ${src_list})
    source_group(3rdparty/base/sqlite-3.36.0 FILES ${sqlite_src_list})
endif()

set(base_database_src_list)
get_cxx_files(${base_database_path}/database src_list)
list(APPEND base_database_src_list ${src_list})
source_group(3rdparty/base/database FILES ${base_database_src_list})

set(base_database_files) # 工程中需要把database的文件添加进去
if(enable_database_crypto)
    list(APPEND base_database_files ${sqlcipher_src_list})
else()
    list(APPEND base_database_files ${sqlite_src_list})
endif()
list(APPEND base_database_files ${base_database_src_list})

print_info(BODY "base database files:")
foreach(filename ${base_database_files})
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
add_executable(example_database ${base_database_files} ${example_files})
