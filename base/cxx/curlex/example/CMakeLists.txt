cmake_minimum_required(VERSION 3.18.0)

project(example_curlex)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加curlex库 ###################################
find_package(CURL 7.76.1 REQUIRED)

if(CURL_FOUND)
    message(STATUS "CURL include path: ${CURL_INCLUDE_DIRS}")
    include_directories(${CURL_INCLUDE_DIRS})
else()
    message(WARNING "CURL not found")
    return()
endif()

set(base_curlex_path ..) # 设置curlex库路径(可修改)
include_directories(${base_curlex_path})

set(base_curlex_src_list)
get_cxx_files(${base_curlex_path}/curlex src_list)
list(APPEND base_curlex_src_list ${src_list})
source_group(3rdparty/base/curlex FILES ${base_curlex_src_list})

set(base_curlex_files) # 工程中需要把curlex的文件添加进去
list(APPEND base_curlex_files ${base_curlex_src_list})

print_info(BODY "base curlex files:")
foreach(filename ${base_curlex_files})
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
add_executable(example_curlex ${base_curlex_files} ${example_files})

# 链接依赖库
target_link_libraries(example_curlex CURL::libcurl)
