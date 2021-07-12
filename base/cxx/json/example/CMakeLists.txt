cmake_minimum_required(VERSION 3.18.0)

project(example_json)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加json库 ###################################
set(json_path ..) # 设置json库路径(可修改)
include_directories(${json_path})

set(nlohmann_src_list)
get_cxx_files(${json_path}/nlohmann src_list)
list(APPEND nlohmann_src_list ${src_list})
source_group(3rdparty/nlohmann FILES ${nlohmann_src_list})

set(json_files) # 工程中需要把json的文件添加进去
list(APPEND json_files ${nlohmann_src_list})

print_info(BODY "json files:")
foreach(filename ${json_files})
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
add_executable(example_json ${json_files} ${example_files})
