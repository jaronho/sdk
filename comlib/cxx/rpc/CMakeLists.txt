cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14)

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源目录]
${comlib_rpc_dir}

[源文件]
${comlib_rpc_broker_files}

[源文件]
${comlib_rpc_client_files}
]]

#################################### 添加rpc模块 ###################################
set(comlib_rpc_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

# 组装文件列表
set(comlib_rpc_broker_files
    ${CMAKE_CURRENT_SOURCE_DIR}/rpc/rpc_broker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rpc/rpc_broker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/rpc/rpc_msg.hpp
    CACHE INTERNAL "")

set(comlib_rpc_client_files
    ${CMAKE_CURRENT_SOURCE_DIR}/rpc/rpc_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rpc/rpc_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/rpc/rpc_msg.hpp
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "comlib rpc broker files:")
foreach(filename ${comlib_rpc_broker_files})
    message("    " ${filename})
endforeach()

print_info(BODY "comlib rpc client files:")
foreach(filename ${comlib_rpc_client_files})
    message("    " ${filename})
endforeach()
