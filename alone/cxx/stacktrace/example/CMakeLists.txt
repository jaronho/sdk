cmake_minimum_required(VERSION 3.18.0)

project(example_stacktrace)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加stacktrace库 ###################################
set(stacktrace_path ..) # 设置stacktrace库路径(可修改)
include_directories(${stacktrace_path})

set(stacktrace_src_list)
get_cxx_files(${stacktrace_path}/stacktrace src_list)
list(APPEND stacktrace_src_list ${src_list})
source_group(3rdparty/stacktrace FILES ${stacktrace_src_list})

set(stacktrace_files)
list(APPEND stacktrace_files ${stacktrace_src_list})

print_info(BODY "stacktrace files:") # 工程中需要把stacktrace的文件添加进去
foreach(filename ${stacktrace_files})
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
add_executable(example_stacktrace ${stacktrace_files} ${example_files})

# 链接依赖库
target_link_libraries(example_stacktrace)
