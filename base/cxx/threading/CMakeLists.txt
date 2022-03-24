cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14)

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[命令行]
find_package(Threads REQUIRED)

[命令行]
set(Boost_USE_STATIC_LIBS ON) # ON表示链接Boost静态库, OFF或者不设置表示链接动态库
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

[链接库]
Threads::Threads

[链接库]
${Boost_LIBRARIES}
]]

#########################################[ 输出 ]#########################################
#[[
[源目录]
${base_threading_dir}

[源文件]
${base_threading_files}
]]

##################################### 添加threading库 ####################################
set(base_threading_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(base_threading_asio_src_list)
get_cxx_files(threading/asio src_list)
list(APPEND base_threading_asio_src_list ${src_list})

set(base_threading_diagnose_src_list)
get_cxx_files(threading/diagnose src_list)
list(APPEND base_threading_diagnose_src_list ${src_list})

set(base_threading_fiber_src_list)
if(enable_fiber)
    get_cxx_files(threading/fiber src_list)
    list(APPEND base_threading_fiber_src_list ${src_list})
endif()

set(base_threading_signal_src_list)
get_cxx_files(threading/signal src_list)
list(APPEND base_threading_signal_src_list ${src_list})

set(base_threading_task_src_list)
get_cxx_files(threading/task src_list)
list(APPEND base_threading_task_src_list ${src_list})

set(base_threading_timer_src_list)
get_cxx_files(threading/timer src_list)
list(APPEND base_threading_timer_src_list ${src_list})

set(base_threading_src_list)
get_cxx_files(threading src_list)
list(APPEND base_threading_src_list ${src_list})

# 组装文件列表
set(base_threading_files
    ${base_threading_asio_src_list}
    ${base_threading_diagnose_src_list}
    ${base_threading_fiber_src_list}
    ${base_threading_signal_src_list}
    ${base_threading_task_src_list}
    ${base_threading_timer_src_list}
    ${base_threading_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base threading files:")
foreach(filename ${base_threading_files})
    message("    " ${filename})
endforeach()
