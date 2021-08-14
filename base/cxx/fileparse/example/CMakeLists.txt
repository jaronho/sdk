cmake_minimum_required(VERSION 3.18.0)

project(example_fileparse)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加fileparse库 ###################################
set(base_fileparse_path ..) # 设置fileparse库路径(可修改)
include_directories(${base_fileparse_path})

set(base_fileparse_ini_src_list)
get_cxx_files(${base_fileparse_path}/fileparse/ini src_list)
list(APPEND base_fileparse_ini_src_list ${src_list})
source_group(3rdparty/base/fileparse/ini FILES ${base_fileparse_ini_src_list})

set(base_fileparse_nlohmann_src_list)
get_cxx_files(${base_fileparse_path}/fileparse/nlohmann src_list)
list(APPEND base_fileparse_nlohmann_src_list ${src_list})
source_group(3rdparty/base/fileparse/nlohmann FILES ${base_fileparse_nlohmann_src_list})

set(base_fileparse_files) # 工程中需要把fileparse的文件添加进去
list(APPEND base_fileparse_files ${base_fileparse_ini_src_list}
                                 ${base_fileparse_nlohmann_src_list})

print_info(BODY "base fileparse files:")
foreach(filename ${base_fileparse_files})
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
add_executable(example_fileparse ${base_fileparse_files} ${example_files})
