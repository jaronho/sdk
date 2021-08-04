cmake_minimum_required(VERSION 3.18.0)

project(example_ini)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加inifile库 ###################################
set(base_inifile_path ..) # 设置inifile库路径(可修改)
include_directories(${base_inifile_path})

set(base_inifile_src_list)
get_cxx_files(${base_inifile_path}/inifile src_list)
list(APPEND base_inifile_src_list ${src_list})
source_group(3rdparty/base/inifile FILES ${base_inifile_src_list})

set(base_inifile_files) # 工程中需要把inifile的文件添加进去
list(APPEND base_inifile_files ${base_inifile_src_list})

print_info(BODY "base inifile files:")
foreach(filename ${base_inifile_files})
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
add_executable(example_ini ${base_inifile_files} ${example_files})
