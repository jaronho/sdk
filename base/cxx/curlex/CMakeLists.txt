cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[命令行]
option(use_curl_staticlib "是否使用CURL静态库" OFF) # ON表示链接CURL静态库, OFF或者不设置表示链接动态库(目前只支持Windows)
find_package(CURL 7.76.1 REQUIRED)
if(CURL_FOUND) # 注意: curl库区分ssl和非ssl
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        if(use_curl_staticlib)
            add_definitions("-DCURL_STATICLIB") # 如果链接静态库则必须添加这个宏
        endif()
        set(CURL_INCLUDE_DIRS $ENV{CURL_HOME}/include) # 读取环境变量`CURL_HOME`的值
    endif()
    set(CURL_LIBRARY_DIRS ${CURL_INCLUDE_DIRS}/../lib)
    message(STATUS "CURL include path: ${CURL_INCLUDE_DIRS}")
    message(STATUS "CURL library path: ${CURL_LIBRARY_DIRS}")
    include_directories(${CURL_INCLUDE_DIRS})
    link_directories(${CURL_LIBRARY_DIRS})
else()
    message(WARNING "CURL not found")
    return()
endif()

#########################################[ 输出 ]#########################################
#[[
[链接库]
${curl_libraries}

[源目录]
${base_curlex_dir}

[源文件]
${base_curlex_files}
]]

##################################### 添加CURL库 #####################################
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    if(use_curl_staticlib) # 设置Debug和Release链接不同的静态库
        set(curl_libraries debug libcurl-d.lib optimized libcurl.lib winmm.lib wldap32.lib ws2_32.lib CACHE INTERNAL "")
    else()
        set(curl_libraries CURL::libcurl winmm.lib wldap32.lib ws2_32.lib CACHE INTERNAL "")
    endif()
else()
    set(curl_libraries CURL::libcurl CACHE INTERNAL "")
endif()

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
