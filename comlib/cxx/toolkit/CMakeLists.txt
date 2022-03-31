cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源目录]
${comlib_toolkit_dir}

[源文件]
${comlib_toolkit_files}
${comlib_toolkit_app_singleton_files}
${comlib_toolkit_net_config_files}
${comlib_toolkit_tool_files}
]]

###################################### 添加toolkit模块 ###################################
set(comlib_toolkit_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(comlib_toolkit_app_singleton_files
    ${CMAKE_CURRENT_SOURCE_DIR}/toolkit/app_singleton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/toolkit/app_singleton.h
    CACHE INTERNAL "")

set(comlib_toolkit_net_config_files
    ${CMAKE_CURRENT_SOURCE_DIR}/toolkit/net_config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/toolkit/net_config.h
    CACHE INTERNAL "")

set(comlib_toolkit_tool_files
    ${CMAKE_CURRENT_SOURCE_DIR}/toolkit/tool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/toolkit/tool.h
    CACHE INTERNAL "")

# 组装文件列表
set(comlib_toolkit_files
    ${comlib_toolkit_app_singleton_files}
    ${comlib_toolkit_net_config_files}
    ${comlib_toolkit_tool_files}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "comlib toolkit files:")
foreach(filename ${comlib_toolkit_files})
    message("    " ${filename})
endforeach()
