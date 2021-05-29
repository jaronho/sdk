cmake_minimum_required(VERSION 3.18.0)

project(example_crashdump)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
##########################################################################################

################################### 添加breakpad库 ###################################
set(breakpad_libraries)
list(APPEND breakpad_libraries breakpad_client)
##########################################################################################

################################### 添加crashdump库 ###################################
set(crashdump_path ..) # 设置crashdump库路径(可修改)
include_directories(${crashdump_path})

set(crashdump_src_list)
get_cxx_files(${crashdump_path}/crashdump src_list)
list(APPEND crashdump_src_list ${src_list})
source_group(3rdparty/crashdump FILES ${crashdump_src_list})

set(crashdump_files)
list(APPEND crashdump_files ${crashdump_src_list})

print_info(BODY "crashdump files:") # 工程中需要把crashdump的文件添加进去
foreach(filename ${crashdump_files})
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
add_executable(example_crashdump ${crashdump_files} ${example_files})

# 链接依赖库
target_link_libraries(example_crashdump Threads::Threads ${breakpad_libraries})
