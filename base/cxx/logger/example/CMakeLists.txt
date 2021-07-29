cmake_minimum_required(VERSION 3.18.0)

project(example_logger)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加logger库 ###################################
option(ENABLE_LOGGER_DETAIL "是否启用日志详情(包含文件名,行号,函数名)" ON)

if(ENABLE_LOGGER_DETAIL)
    add_definitions(-DENABLE_LOGGER_DETAIL=2)
else()
    add_definitions(-DENABLE_LOGGER_DETAIL=0)
endif()

set(logger_path ..) # 设置logger库路径(可修改)
include_directories(${logger_path})
include_directories(${logger_path}/fmt-7.1.3/include)

set(fmt_src_list)
get_cxx_files(${logger_path}/fmt-7.1.3/src src_list)
list(APPEND fmt_src_list ${src_list})
source_group(3rdparty/fmt FILES ${fmt_src_list})

set(logger_impl_src_list)
get_cxx_files(${logger_path}/logger/impl src_list)
list(APPEND logger_impl_src_list ${src_list})
source_group(3rdparty/logger/impl FILES ${logger_impl_src_list})

set(logger_logfile_src_list)
get_cxx_files(${logger_path}/logger/logfile src_list)
list(APPEND logger_logfile_src_list ${src_list})
source_group(3rdparty/logger/logfile FILES ${logger_logfile_src_list})

set(logger_src_list)
get_cxx_files(${logger_path}/logger src_list)
list(APPEND logger_src_list ${src_list})
source_group(3rdparty/logger FILES ${logger_src_list})

set(logger_files) # 工程中需要把logger的文件添加进去
list(APPEND logger_files ${fmt_src_list} ${logger_impl_src_list} ${logger_logfile_src_list} ${logger_src_list})

print_info(BODY "logger files:")
foreach(filename ${logger_files})
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
add_executable(example_logger ${logger_files} ${example_files})
