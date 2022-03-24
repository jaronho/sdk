cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14)

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[开关]
option(enable_logger_detail "是否启用日志详情(包含文件名,行号,函数名)" ON)
if(enable_logger_detail)
    add_definitions(-DENABLE_LOGGER_DETAIL=2)
else()
    add_definitions(-DENABLE_LOGGER_DETAIL=0)
endif()
]]

#########################################[ 输出 ]#########################################
#[[
[源目录]
${fmt_include_dir}

[源文件]
${base_logger_files}
]]

###################################### 添加logger库 ######################################
set(fmt_include_dir fmt-7.1.3/include CACHE INTERNAL "")

set(base_fmt_src_list)
get_cxx_files(${fmt_include_dir}/fmt src_list)
list(APPEND base_fmt_src_list ${src_list})
get_cxx_files(${fmt_include_dir}/../src src_list)
list(APPEND base_fmt_src_list ${src_list})

set(base_logger_impl_src_list)
get_cxx_files(logger/impl src_list)
list(APPEND base_logger_impl_src_list ${src_list})

set(base_logger_logfile_src_list)
get_cxx_files(logger/logfile src_list)
list(APPEND base_logger_logfile_src_list ${src_list})

set(base_logger_src_list)
get_cxx_files(logger src_list)
list(APPEND base_logger_src_list ${src_list})

# 组装文件列表
set(base_logger_files
    ${base_fmt_src_list}
    ${base_logger_impl_src_list}
    ${base_logger_logfile_src_list}
    ${base_logger_src_list}
    CACHE INTERNAL "") 

# 打印文件列表
print_info(BODY "base logger files:")
foreach(filename ${base_logger_files})
    message("    " ${filename})
endforeach()
