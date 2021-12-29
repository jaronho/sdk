cmake_minimum_required(VERSION 3.18.0)

project(example_serial)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加serial库 ###################################
set(base_serial_path ..) # 设置serial库路径(可修改)
include_directories(${base_serial_path})

set(base_serial_impl_list_ports_src_list)
get_cxx_files(${base_serial_path}/serial/impl/list_ports src_list)
list(APPEND base_serial_impl_list_ports_src_list ${src_list})
source_group(3rdparty/base/serial/impl/list_ports FILES ${base_serial_impl_list_ports_src_list})

set(base_serial_impl_src_list)
get_cxx_files(${base_serial_path}/serial/impl src_list)
list(APPEND base_serial_impl_src_list ${src_list})
source_group(3rdparty/base/serial/impl FILES ${base_serial_impl_src_list})

set(base_serial_src_list)
get_cxx_files(${base_serial_path}/serial src_list)
list(APPEND base_serial_src_list ${src_list})
source_group(3rdparty/base/serial FILES ${base_serial_src_list})

set(base_serial_files) # 工程中需要把serial的文件添加进去
list(APPEND base_serial_files ${base_serial_impl_list_ports_src_list}
                              ${base_serial_impl_src_list}
                              ${base_serial_src_list})

print_info(BODY "base serial files:")
foreach(filename ${base_serial_files})
    message("    " ${filename})
endforeach()
##########################################################################################

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
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
add_executable(example_serial ${base_serial_files} ${example_files})

# 链接依赖库文件
target_link_libraries(example_serial Threads::Threads)
