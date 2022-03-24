cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源目录]
${base_serial_dir}

[源文件]
${base_serial_files}
]]

###################################### 添加serial库 ######################################
set(base_serial_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(base_serial_impl_list_ports_src_list)
get_cxx_files(serial/impl/list_ports src_list)
list(APPEND base_serial_impl_list_ports_src_list ${src_list})

set(base_serial_impl_src_list)
get_cxx_files(serial/impl src_list)
list(APPEND base_serial_impl_src_list ${src_list})

set(base_serial_src_list)
get_cxx_files(serial src_list)
list(APPEND base_serial_src_list ${src_list})

# 组装文件列表
set(base_serial_files
    ${base_serial_impl_list_ports_src_list}
    ${base_serial_impl_src_list}
    ${base_serial_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base serial files:")
foreach(filename ${base_serial_files})
    message("    " ${filename})
endforeach()
