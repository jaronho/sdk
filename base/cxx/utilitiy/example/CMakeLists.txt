cmake_minimum_required(VERSION 3.18.0)

project(example_utilitiy)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
##########################################################################################

################################### 添加utilitiy库 ###################################
set(utilitiy_path ..) # 设置utilitiy库路径(可修改)
include_directories(${utilitiy_path})

set(utilitiy_cmdline_src_list)
get_cxx_files(${utilitiy_path}/utilitiy/cmdline src_list)
list(APPEND utilitiy_cmdline_src_list ${src_list})
source_group(3rdparty/utilitiy/cmdline FILES ${utilitiy_cmdline_src_list})

set(utilitiy_filesystem_src_list)
get_cxx_files(${utilitiy_path}/utilitiy/filesystem src_list)
list(APPEND utilitiy_filesystem_src_list ${src_list})
source_group(3rdparty/utilitiy/filesystem FILES ${utilitiy_filesystem_src_list})

set(utilitiy_process_src_list)
get_cxx_files(${utilitiy_path}/utilitiy/process src_list)
list(APPEND utilitiy_process_src_list ${src_list})
source_group(3rdparty/utilitiy/process FILES ${utilitiy_process_src_list})

set(utilitiy_system_src_list)
get_cxx_files(${utilitiy_path}/utilitiy/system src_list)
list(APPEND utilitiy_system_src_list ${src_list})
source_group(3rdparty/utilitiy/system FILES ${utilitiy_system_src_list})

set(utilitiy_src_list)
get_cxx_files(${utilitiy_path}/utilitiy src_list)
list(APPEND utilitiy_src_list ${src_list})
source_group(3rdparty/utilitiy FILES ${utilitiy_src_list})

set(utilitiy_files) # 工程中需要把utilitiy的文件添加进去
list(APPEND utilitiy_files ${utilitiy_cmdline_src_list}
                           ${utilitiy_filesystem_src_list}
                           ${utilitiy_process_src_list}
                           ${utilitiy_system_src_list}
                           ${utilitiy_src_list})

print_info(BODY "utilitiy files:")
foreach(filename ${utilitiy_files})
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
add_executable(example_utilitiy ${utilitiy_files} ${example_files})

# 链接依赖库
target_link_libraries(example_utilitiy Threads::Threads)
