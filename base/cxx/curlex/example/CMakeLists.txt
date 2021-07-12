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

set(curlex_path ..) # 设置curlex库路径(可修改)
include_directories(${curlex_path})

set(curlex_src_list)
get_cxx_files(${curlex_path}/curlex src_list)
list(APPEND curlex_src_list ${src_list})
source_group(3rdparty/curlex FILES ${curlex_src_list})

set(curlex_files) # 工程中需要把curlex的文件添加进去
list(APPEND curlex_files ${curlex_src_list})

print_info(BODY "curlex files:")
foreach(filename ${curlex_files})
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
add_executable(example_curlex ${curlex_files} ${example_files})

# 链接依赖库
target_link_libraries(example_curlex CURL::libcurl)
