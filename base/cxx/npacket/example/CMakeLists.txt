cmake_minimum_required(VERSION 3.18.0)

project(example_npacket)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
##########################################################################################

################################### 添加npacket库 ###################################
set(base_npacket_path ..) # 设置npacket库路径(可修改)
include_directories(${base_npacket_path})

set(base_npacket_src_list)
get_cxx_files(${base_npacket_path}/npacket src_list)
list(APPEND base_npacket_src_list ${src_list})
source_group(3rdparty/base/npacket FILES ${base_npacket_src_list})

set(base_npacket_files) # 工程中需要把npacket的文件添加进去
list(APPEND base_npacket_files ${base_npacket_src_list})

print_info(BODY "base npacket files:")
foreach(filename ${base_npacket_files})
    message("    " ${filename})
endforeach()
##########################################################################################

# 添加客户端示例文件
set(example_files)
get_cxx_files(. src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_npacket ${base_npacket_files} ${example_files})

# 链接依赖库
target_link_libraries(example_npacket Threads::Threads)
