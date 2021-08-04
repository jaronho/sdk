cmake_minimum_required(VERSION 3.18.0)

project(example_httpclient)

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

set(base_threading_path ../../../../3rdparty/cxx_base_threading) # 设置threading库路径(可修改)
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

################################### 添加curlex库 ###################################
find_package(CURL 7.76.1 REQUIRED)

if(CURL_FOUND)
    message(STATUS "CURL include path: ${CURL_INCLUDE_DIRS}")
    include_directories(${CURL_INCLUDE_DIRS})
else()
    message(WARNING "CURL not found")
    return()
endif()

set(base_curlex_path ../../../../3rdparty/cxx_base_curlex) # 设置curlex库路径(可修改)
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

################################### 添加httpclient模块 ###################################
set(httpclient_path ..) # 设置httpclient模块路径(可修改)
include_directories(${httpclient_path})

set(httpclient_src_list)
get_cxx_files(${httpclient_path}/httpclient src_list)
list(APPEND httpclient_src_list ${src_list})
source_group(common/httpclient FILES ${httpclient_src_list})

set(httpclient_files) # 工程中需要把httpclient的文件添加进去
list(APPEND httpclient_files ${httpclient_src_list})

print_info(BODY "httpclient files:")
foreach(filename ${httpclient_files})
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
add_executable(example_httpclient ${base_threading_files} ${base_curlex_files} ${httpclient_files} ${example_files})

# 链接依赖库
target_link_libraries(example_httpclient Threads::Threads ${Boost_LIBRARIES} CURL::libcurl)
