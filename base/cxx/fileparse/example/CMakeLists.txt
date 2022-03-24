cmake_minimum_required(VERSION 3.18.0)

project(example_fileparse)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

# 添加fileparse基础库
add_subdirectory(${CMAKE_SOURCE_DIR}/.. 3rdparty/base/fileparse)
include_directories(${CMAKE_SOURCE_DIR}/..)
source_group(TREE ${CMAKE_SOURCE_DIR}/.. PREFIX 3rdparty/base FILES ${base_fileparse_files})

# 添加示例文件
set(example_files)
get_cxx_files(. src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_fileparse ${base_fileparse_files} ${example_files})
