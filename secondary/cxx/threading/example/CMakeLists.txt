cmake_minimum_required(VERSION 3.18.0)

project(example_threading)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
# Fix for PThread library not in path
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
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

set(threading_path ..) # 设置threading库路径(可修改)
include_directories(${threading_path})

set(threading_asio_src_list)
get_cxx_files(${threading_path}/threading/asio src_list)
list(APPEND threading_asio_src_list ${src_list})
source_group(3rdparty/threading/asio FILES ${threading_asio_src_list})

set(threading_diagnose_src_list)
get_cxx_files(${threading_path}/threading/diagnose src_list)
list(APPEND threading_diagnose_src_list ${src_list})
source_group(3rdparty/threading/diagnose FILES ${threading_diagnose_src_list})

set(threading_fiber_src_list)
if(enable_fiber)
    get_cxx_files(${threading_path}/threading/fiber src_list)
    list(APPEND threading_fiber_src_list ${src_list})
    source_group(3rdparty/threading/fiber FILES ${threading_fiber_src_list})
endif()

set(threading_task_src_list)
get_cxx_files(${threading_path}/threading/task src_list)
list(APPEND threading_task_src_list ${src_list})
source_group(3rdparty/threading/task FILES ${threading_task_src_list})

set(threading_src_list)
get_cxx_files(${threading_path}/threading src_list)
list(APPEND threading_src_list ${src_list})
source_group(3rdparty/threading FILES ${threading_src_list})

set(threading_files) # 工程中需要把threading的文件添加进去
list(APPEND threading_files ${threading_asio_src_list} ${threading_diagnose_src_list} ${threading_fiber_src_list} ${threading_task_src_list} ${threading_src_list})

print_info(BODY "threading files:")
foreach(filename ${threading_files})
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
add_executable(example_threading ${threading_files} ${example_files})

# 链接依赖库
target_link_libraries(example_threading Threads::Threads ${Boost_LIBRARIES})
