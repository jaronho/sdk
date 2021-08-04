cmake_minimum_required(VERSION 3.18.0)

project(example_timeconsuming)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加timeconsuming库 ###################################
set(base_timeconsuming_path ..) # 设置timeconsuming库路径(可修改)
include_directories(${base_timeconsuming_path})

set(base_timeconsuming_src_list)
get_cxx_files(${base_timeconsuming_path}/timeconsuming src_list)
list(APPEND base_timeconsuming_src_list ${src_list})
source_group(3rdparty/base/timeconsuming FILES ${base_timeconsuming_src_list})

set(base_timeconsuming_files) # 工程中需要把timeconsuming的文件添加进去
list(APPEND base_timeconsuming_files ${base_timeconsuming_src_list})

print_info(BODY "base timeconsuming files:")
foreach(filename ${base_timeconsuming_files})
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
add_executable(example_timeconsuming ${base_timeconsuming_files} ${example_files})
