cmake_minimum_required(VERSION 3.18.0)

project(example_ini)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加inifile库 ###################################
set(inifile_path ..) # 设置inifile库路径(可修改)
include_directories(${inifile_path})

set(inifile_src_list)
get_cxx_files(${inifile_path}/inifile src_list)
list(APPEND inifile_src_list ${src_list})
source_group(3rdparty/inifile FILES ${inifile_src_list})

set(inifile_files) # 工程中需要把inifile的文件添加进去
list(APPEND inifile_files ${inifile_src_list})

print_info(BODY "inifile files:")
foreach(filename ${inifile_files})
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
add_executable(example_ini ${inifile_files} ${example_files})
