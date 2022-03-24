cmake_minimum_required(VERSION 3.18.0)

project(example_curlex)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

####################################### 添加CURL库 #######################################
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
##########################################################################################

# 添加curlex基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/curlex)
include_directories(${base_curlex_dir})
source_group(TREE ${base_curlex_dir} PREFIX 3rdparty/base FILES ${base_curlex_files})

# 添加示例文件
set(example_files)
get_cxx_files(${CMAKE_CURRENT_SOURCE_DIR} src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_curlex ${base_curlex_files} ${example_files})

# 链接依赖库
target_link_libraries(example_curlex CURL::libcurl)
