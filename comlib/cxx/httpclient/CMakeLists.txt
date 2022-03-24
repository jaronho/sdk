cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14)

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源目录]
${comlib_httpclient_dir}

[源文件]
${comlib_httpclient_files}
]]

#################################### 添加httpclient模块 ###################################
set(comlib_httpclient_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(comlib_httpclient_src_list)
get_cxx_files(httpclient src_list)
list(APPEND comlib_httpclient_src_list ${src_list})

# 组装文件列表
set(comlib_httpclient_files
    ${comlib_httpclient_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "comlib httpclient files:")
foreach(filename ${comlib_httpclient_files})
    message("    " ${filename})
endforeach()
