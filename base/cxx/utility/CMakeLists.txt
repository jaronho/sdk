cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[命令行]
find_package(Threads REQUIRED)

[链接库]
Threads::Threads
]]

#########################################[ 输出 ]#########################################
#[[
[源目录]
${base_utility_dir}

[源文件]
${base_utility_files}
]]

################################### 添加utility库 ###################################
set(base_utility_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(base_utility_bytearray_src_list)
get_cxx_files(utility/bytearray src_list)
list(APPEND base_utility_bytearray_src_list ${src_list})

set(base_utility_charset_src_list)
get_cxx_files(utility/charset src_list)
list(APPEND base_utility_charset_src_list ${src_list})

set(base_utility_cmdline_src_list)
get_cxx_files(utility/cmdline src_list)
list(APPEND base_utility_cmdline_src_list ${src_list})

set(base_utility_datetime_src_list)
get_cxx_files(utility/datetime src_list)
list(APPEND base_utility_datetime_src_list ${src_list})

set(base_utility_digit_src_list)
get_cxx_files(utility/digit src_list)
list(APPEND base_utility_digit_src_list ${src_list})

set(base_utility_filesystem_src_list)
get_cxx_files(utility/filesystem src_list)
list(APPEND base_utility_filesystem_src_list ${src_list})

set(base_utility_module_src_list)
get_cxx_files(utility/module src_list)
list(APPEND base_utility_module_src_list ${src_list})

set(base_utility_net_src_list)
get_cxx_files(utility/net src_list)
list(APPEND base_utility_net_src_list ${src_list})

set(base_utility_process_src_list)
get_cxx_files(utility/process src_list)
list(APPEND base_utility_process_src_list ${src_list})

set(base_utility_strtool_src_list)
get_cxx_files(utility/strtool src_list)
list(APPEND base_utility_strtool_src_list ${src_list})

set(base_utility_system_src_list)
get_cxx_files(utility/system src_list)
list(APPEND base_utility_system_src_list ${src_list})

set(base_utility_timewatch_src_list)
get_cxx_files(utility/timewatch src_list)
list(APPEND base_utility_timewatch_src_list ${src_list})

set(base_utility_src_list)
get_cxx_files(utility src_list)
list(APPEND base_utility_src_list ${src_list})

# 组装文件列表
set(base_utility_files
    ${base_utility_bytearray_src_list}
    ${base_utility_charset_src_list}
    ${base_utility_cmdline_src_list}
    ${base_utility_datetime_src_list}
    ${base_utility_digit_src_list}
    ${base_utility_filesystem_src_list}
    ${base_utility_module_src_list}
    ${base_utility_net_src_list}
    ${base_utility_process_src_list}
    ${base_utility_strtool_src_list}
    ${base_utility_system_src_list}
    ${base_utility_timewatch_src_list}
    ${base_utility_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base utility files:")
foreach(filename ${base_utility_files})
    message("    " ${filename})
endforeach()
