cmake_minimum_required(VERSION 3.18.0)

project(example_timewatch)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加timewatch库 ###################################
set(base_timewatch_path ..) # 设置timewatch库路径(可修改)
include_directories(${base_timewatch_path})

set(base_timewatch_src_list)
get_cxx_files(${base_timewatch_path}/timewatch src_list)
list(APPEND base_timewatch_src_list ${src_list})
source_group(3rdparty/base/timewatch FILES ${base_timewatch_src_list})

set(base_timewatch_files) # 工程中需要把timewatch的文件添加进去
list(APPEND base_timewatch_files ${base_timewatch_src_list})

print_info(BODY "base timewatch files:")
foreach(filename ${base_timewatch_files})
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
add_executable(example_timewatch ${base_timewatch_files} ${example_files})
