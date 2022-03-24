cmake_minimum_required(VERSION 3.18.0)

project(example_crashdump)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

# 添加堆栈崩溃捕获基础库
add_subdirectory(${CMAKE_SOURCE_DIR}/.. 3rdparty/base/crashdump)
include_directories(${CMAKE_SOURCE_DIR}/..)
source_group(TREE ${CMAKE_SOURCE_DIR}/.. PREFIX 3rdparty/base FILES ${base_crashdump_files})

# 添加示例文件
set(example_files)
get_cxx_files(. src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_crashdump ${base_crashdump_files} ${example_files})

# 链接依赖库
target_link_libraries(example_crashdump ${base_crashdump} Threads::Threads ${breakpad_libraries})
