cmake_minimum_required(VERSION 3.18.0)

project(example_module)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加module库 ###################################
set(base_module_path ..) # 设置module库路径(可修改)
include_directories(${base_module_path})

set(base_module_src_list)
get_cxx_files(${base_module_path}/module src_list)
list(APPEND base_module_src_list ${src_list})
source_group(3rdparty/base/module FILES ${base_module_src_list})

set(base_module_files) # 工程中需要把module的文件添加进去
list(APPEND base_module_files ${base_module_src_list})

print_info(BODY "base module files:")
foreach(filename ${base_module_files})
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
add_executable(example_module ${base_module_files} ${example_files})
