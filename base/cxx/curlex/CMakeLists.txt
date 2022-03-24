cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[命令行]
find_package(CURL 7.76.1 REQUIRED)
if(CURL_FOUND) # 注意: curl库区分ssl和非ssl, 根据业务需要自行手动替换
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        add_definitions("-DCURL_STATICLIB") # 如果链接静态库则必须添加这个宏
    endif()
    message(STATUS "CURL include path: ${CURL_INCLUDE_DIRS}")
    include_directories(${CURL_INCLUDE_DIRS})
else()
    message(WARNING "CURL not found")
    return()
endif()

[链接库]
CURL::libcurl
]]

#########################################[ 输出 ]#########################################
#[[
[源目录]
${base_curlex_dir}

[源文件]
${base_curlex_files}
]]

###################################### 添加curlex库 ######################################
set(base_curlex_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(base_curlex_src_list)
get_cxx_files(curlex src_list)
list(APPEND base_curlex_src_list ${src_list})

# 组装文件列表
set(base_curlex_files
    ${base_curlex_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base curlex files:")
foreach(filename ${base_curlex_files})
    message("    " ${filename})
endforeach()
