cmake_minimum_required(VERSION 3.18.0)

project(example_utility)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
##########################################################################################

################################### 添加utility库 ###################################
set(base_utility_path ..) # 设置utility库路径(可修改)
include_directories(${base_utility_path})

set(base_utility_bytearray_src_list)
get_cxx_files(${base_utility_path}/utility/bytearray src_list)
list(APPEND base_utility_bytearray_src_list ${src_list})
source_group(3rdparty/base/utility/bytearray FILES ${base_utility_bytearray_src_list})

set(base_utility_charset_src_list)
get_cxx_files(${base_utility_path}/utility/charset src_list)
list(APPEND base_utility_charset_src_list ${src_list})
source_group(3rdparty/base/utility/charset FILES ${base_utility_charset_src_list})

set(base_utility_cmdline_src_list)
get_cxx_files(${base_utility_path}/utility/cmdline src_list)
list(APPEND base_utility_cmdline_src_list ${src_list})
source_group(3rdparty/base/utility/cmdline FILES ${base_utility_cmdline_src_list})

set(base_utility_datetime_src_list)
get_cxx_files(${base_utility_path}/utility/datetime src_list)
list(APPEND base_utility_datetime_src_list ${src_list})
source_group(3rdparty/base/utility/datetime FILES ${base_utility_datetime_src_list})

set(base_utility_digit_src_list)
get_cxx_files(${base_utility_path}/utility/digit src_list)
list(APPEND base_utility_digit_src_list ${src_list})
source_group(3rdparty/base/utility/digit FILES ${base_utility_digit_src_list})

set(base_utility_filesystem_src_list)
get_cxx_files(${base_utility_path}/utility/filesystem src_list)
list(APPEND base_utility_filesystem_src_list ${src_list})
source_group(3rdparty/base/utility/filesystem FILES ${base_utility_filesystem_src_list})

set(base_utility_module_src_list)
get_cxx_files(${base_utility_path}/utility/module src_list)
list(APPEND base_utility_module_src_list ${src_list})
source_group(3rdparty/base/utility/module FILES ${base_utility_module_src_list})

set(base_utility_net_src_list)
get_cxx_files(${base_utility_path}/utility/net src_list)
list(APPEND base_utility_net_src_list ${src_list})
source_group(3rdparty/base/utility/net FILES ${base_utility_net_src_list})

set(base_utility_process_src_list)
get_cxx_files(${base_utility_path}/utility/process src_list)
list(APPEND base_utility_process_src_list ${src_list})
source_group(3rdparty/base/utility/process FILES ${base_utility_process_src_list})

set(base_utility_strtool_src_list)
get_cxx_files(${base_utility_path}/utility/strtool src_list)
list(APPEND base_utility_strtool_src_list ${src_list})
source_group(3rdparty/base/utility/strtool FILES ${base_utility_strtool_src_list})

set(base_utility_system_src_list)
get_cxx_files(${base_utility_path}/utility/system src_list)
list(APPEND base_utility_system_src_list ${src_list})
source_group(3rdparty/base/utility/system FILES ${base_utility_system_src_list})

set(base_utility_timewatch_src_list)
get_cxx_files(${base_utility_path}/utility/timewatch src_list)
list(APPEND base_utility_timewatch_src_list ${src_list})
source_group(3rdparty/base/utility/timewatch FILES ${base_utility_timewatch_src_list})

set(base_utility_src_list)
get_cxx_files(${base_utility_path}/utility src_list)
list(APPEND base_utility_src_list ${src_list})
source_group(3rdparty/base/utility FILES ${base_utility_src_list})

set(base_utility_files) # 工程中需要把utility的文件添加进去
list(APPEND base_utility_files ${base_utility_bytearray_src_list}
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
                                ${base_utility_src_list})

print_info(BODY "base utility files:")
foreach(filename ${base_utility_files})
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
add_executable(example_utility ${base_utility_files} ${example_files})

# 链接依赖库
target_link_libraries(example_utility Threads::Threads)
