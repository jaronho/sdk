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

[源文件]
${base_utility_bytearray_files}

[源文件]
${base_utility_charset_files}

[源文件]
${base_utility_cmdline_files}

[源文件]
${base_utility_dataclass_files}

[源文件]
${base_utility_datetime_files}

[源文件]
${base_utility_digit_files}

[源文件]
${base_utility_filesystem_files}

[源文件]
${base_utility_mmfile_files}

[源文件]
${base_utility_module_files}

[源文件]
${base_utility_net_files}

[源文件]
${base_utility_process_files}

[源文件]
${base_utility_strtool_files}

[源文件]
${base_utility_system_files}

[源文件]
${base_utility_timewatch_files}

[源文件]
${base_utility_util_files}
]]

################################### 添加utility库 ###################################
set(base_utility_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

get_cxx_files(utility/bytearray src_list)
set(base_utility_bytearray_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/charset src_list)
set(base_utility_charset_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/cmdline src_list)
set(base_utility_cmdline_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/dataclass src_list)
set(base_utility_dataclass_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/datetime src_list)
set(base_utility_datetime_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/digit src_list)
set(base_utility_digit_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/filesystem src_list)
set(base_utility_filesystem_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/mmfile src_list)
set(base_utility_mmfile_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/module src_list)
set(base_utility_module_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/net src_list)
set(base_utility_net_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/process src_list)
set(base_utility_process_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/strtool src_list)
set(base_utility_strtool_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/system src_list)
set(base_utility_system_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/timewatch src_list)
set(base_utility_timewatch_files ${src_list} CACHE INTERNAL "")

get_cxx_files(utility/util src_list)
set(base_utility_util_files ${src_list} CACHE INTERNAL "")

# 组装文件列表
set(base_utility_files
    ${base_utility_bytearray_files}
    ${base_utility_charset_files}
    ${base_utility_cmdline_files}
    ${base_utility_dataclass_files}
    ${base_utility_datetime_files}
    ${base_utility_digit_files}
    ${base_utility_filesystem_files}
    ${base_utility_mmfile_files}
    ${base_utility_module_files}
    ${base_utility_net_files}
    ${base_utility_process_files}
    ${base_utility_strtool_files}
    ${base_utility_system_files}
    ${base_utility_timewatch_files}
    ${base_utility_util_files}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base utility files:")
foreach(filename ${base_utility_files})
    message("    " ${filename})
endforeach()
