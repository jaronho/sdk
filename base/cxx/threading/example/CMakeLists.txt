cmake_minimum_required(VERSION 3.18.0)

project(example_threading)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
##########################################################################################

################################### 添加threading库 ###################################
option(enable_fiber "是否启用Boost库的Fiber模块" ON)

if(enable_fiber)
    find_package(Boost 1.76.0 REQUIRED COMPONENTS date_time fiber)
    add_definitions(-DENABLE_THREADING_FIBER=1)
else()
    find_package(Boost 1.76.0 REQUIRED COMPONENTS date_time)
    add_definitions(-DENABLE_THREADING_FIBER=0)
endif()

if(Boost_FOUND)
    message(STATUS "Boost include path: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost library path: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(WARNING "Boost not found")
    return()
endif()

set(base_threading_path ..) # 设置threading库路径(可修改)
include_directories(${base_threading_path})

set(base_threading_asio_src_list)
get_cxx_files(${base_threading_path}/threading/asio src_list)
list(APPEND base_threading_asio_src_list ${src_list})
source_group(3rdparty/base/threading/asio FILES ${base_threading_asio_src_list})

set(base_threading_diagnose_src_list)
get_cxx_files(${base_threading_path}/threading/diagnose src_list)
list(APPEND base_threading_diagnose_src_list ${src_list})
source_group(3rdparty/base/threading/diagnose FILES ${base_threading_diagnose_src_list})

set(base_threading_fiber_src_list)
if(enable_fiber)
    get_cxx_files(${base_threading_path}/threading/fiber src_list)
    list(APPEND base_threading_fiber_src_list ${src_list})
    source_group(3rdparty/base/threading/fiber FILES ${base_threading_fiber_src_list})
endif()

set(base_threading_task_src_list)
get_cxx_files(${base_threading_path}/threading/task src_list)
list(APPEND base_threading_task_src_list ${src_list})
source_group(3rdparty/base/threading/task FILES ${base_threading_task_src_list})

set(base_threading_src_list)
get_cxx_files(${base_threading_path}/threading src_list)
list(APPEND base_threading_src_list ${src_list})
source_group(3rdparty/base/threading FILES ${base_threading_src_list})

set(base_threading_files) # 工程中需要把threading的文件添加进去
list(APPEND base_threading_files ${base_threading_asio_src_list}
                                 ${base_threading_diagnose_src_list}
                                 ${base_threading_fiber_src_list}
                                 ${base_threading_task_src_list}
                                 ${base_threading_src_list})

print_info(BODY "base threading files:")
foreach(filename ${base_threading_files})
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
add_executable(example_threading ${base_threading_files} ${example_files})

# 链接依赖库
target_link_libraries(example_threading Threads::Threads ${Boost_LIBRARIES})
