cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源目录]
${comlib_toolkit_dir}

[源文件]
${comlib_toolkit_files}
]]

###################################### 添加toolkit模块 ###################################
set(comlib_toolkit_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(comlib_toolkit_src_list)
get_cxx_files(toolkit src_list)
list(APPEND comlib_toolkit_src_list ${src_list})

# 组装文件列表
set(comlib_toolkit_files
    ${comlib_toolkit_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "comlib toolkit files:")
foreach(filename ${comlib_toolkit_files})
    message("    " ${filename})
endforeach()
