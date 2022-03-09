cmake_minimum_required(VERSION 3.18.0)

project(example_logger)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加logger库 ###################################
option(enable_logger_detail "是否启用日志详情(包含文件名,行号,函数名)" ON)

if(enable_logger_detail)
    add_definitions(-DENABLE_LOGGER_DETAIL=2)
else()
    add_definitions(-DENABLE_LOGGER_DETAIL=0)
endif()

set(base_logger_path ..) # 设置logger库路径(可修改)
include_directories(${base_logger_path})
include_directories(${base_logger_path}/fmt-7.1.3/include)

set(base_fmt_src_list)
get_cxx_files(${base_logger_path}/fmt-7.1.3/include/fmt src_list)
list(APPEND base_fmt_src_list ${src_list})
get_cxx_files(${base_logger_path}/fmt-7.1.3/src src_list)
list(APPEND base_fmt_src_list ${src_list})
source_group(3rdparty/base/fmt FILES ${base_fmt_src_list})

set(base_logger_impl_src_list)
get_cxx_files(${base_logger_path}/logger/impl src_list)
list(APPEND base_logger_impl_src_list ${src_list})
source_group(3rdparty/base/logger/impl FILES ${base_logger_impl_src_list})

set(base_logger_logfile_src_list)
get_cxx_files(${base_logger_path}/logger/logfile src_list)
list(APPEND base_logger_logfile_src_list ${src_list})
source_group(3rdparty/base/logger/logfile FILES ${base_logger_logfile_src_list})

set(base_logger_src_list)
get_cxx_files(${base_logger_path}/logger src_list)
list(APPEND base_logger_src_list ${src_list})
source_group(3rdparty/base/logger FILES ${base_logger_src_list})

set(base_logger_files) # 工程中需要把logger的文件添加进去
list(APPEND base_logger_files ${base_fmt_src_list}
                              ${base_logger_impl_src_list}
                              ${base_logger_logfile_src_list}
                              ${base_logger_src_list})

print_info(BODY "base logger files:")
foreach(filename ${base_logger_files})
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
add_executable(example_logger ${base_logger_files} ${example_files})
