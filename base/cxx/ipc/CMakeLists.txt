cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源目录]
${base_ipc_dir}

[源文件]
${base_ipc_files}

[源文件]
${base_ipc_simdb_files}
]]

##################################### 添加ipc库 ####################################
set(base_ipc_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(base_ipc_simdb_src_list)
get_files(ipc/simdb *.hpp src_list)
list(APPEND base_ipc_simdb_src_list ${src_list})

set(base_ipc_src_list)
get_cxx_files(ipc src_list)
list(APPEND base_ipc_src_list ${src_list})

# 组装文件列表
set(base_ipc_files
    ${base_ipc_simdb_src_list}
    ${base_ipc_src_list}
    CACHE INTERNAL "")

set(base_ipc_simdb_files
    ${base_ipc_simdb_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base ipc files:")
foreach(filename ${base_ipc_files})
    message("    " ${filename})
endforeach()
