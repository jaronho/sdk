cmake_minimum_required(VERSION 3.18.0)

project(example_module)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加module库 ###################################
set(module_path ..) # 设置module库路径(可修改)
include_directories(${module_path})

set(module_src_list)
get_cxx_files(${module_path}/module src_list)
list(APPEND module_src_list ${src_list})
source_group(3rdparty/module FILES ${module_src_list})

set(module_files) # 工程中需要把module的文件添加进去
list(APPEND module_files ${module_src_list})

print_info(BODY "module files:")
foreach(filename ${module_files})
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
add_executable(example_module ${module_files} ${example_files})
