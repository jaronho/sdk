cmake_minimum_required(VERSION 3.18.0)

project(example_httpclient)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

# threading
option(ENABLE_FIBER "是否启用Boost库的Fiber模块" ON)

if(ENABLE_FIBER)
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
endif()

find_package(Threads REQUIRED)

set(THREADING_PATH ../../cxx_base_threading)
include_directories(${THREADING_PATH})

set(THREADING_TASK_SOURCES)
get_cxx_files(${THREADING_PATH}/threading/task SRC_LIST)
list(APPEND THREADING_TASK_SOURCES ${SRC_LIST})
source_group(threading/task FILES ${THREADING_TASK_SOURCES})

set(THREADING_ASIO_SOURCES)
get_cxx_files(${THREADING_PATH}/threading/asio SRC_LIST)
list(APPEND THREADING_ASIO_SOURCES ${SRC_LIST})
source_group(threading/asio FILES ${THREADING_ASIO_SOURCES})

if(ENABLE_FIBER)
    set(THREADING_FIBER_SOURCES)
    get_cxx_files(${THREADING_PATH}/threading/fiber SRC_LIST)
    list(APPEND THREADING_FIBER_SOURCES ${SRC_LIST})
    source_group(threading/fiber FILES ${THREADING_FIBER_SOURCES})
endif()

set(THREADING_DIAGNOSE_SOURCES)
get_cxx_files(${THREADING_PATH}/threading/diagnose SRC_LIST)
list(APPEND THREADING_DIAGNOSE_SOURCES ${SRC_LIST})
source_group(threading/diagnose FILES ${THREADING_DIAGNOSE_SOURCES})

set(THREADING_SOURCES)
get_cxx_files(${THREADING_PATH}/threading SRC_LIST)
list(APPEND THREADING_SOURCES ${SRC_LIST})
source_group(threading FILES ${THREADING_SOURCES})

# curlex
find_package(CURL 7.76.1 REQUIRED)

if(CURL_FOUND)
    message(STATUS "CURL include path: ${CURL_INCLUDE_DIRS}")
    include_directories(${CURL_INCLUDE_DIRS})
else()
    message(WARNING "CURL not found")
endif()

set(CURLEX_PATH ../../cxx_base_curlex)
include_directories(${CURLEX_PATH})

set(CURLEX_SOURCES)
get_cxx_files(${CURLEX_PATH}/curlex SRC_LIST)
list(APPEND CURLEX_SOURCES ${SRC_LIST})
source_group(curlex FILES ${CURLEX_SOURCES})

# httpcliet
set(HTTPCLIENT_PATH ..)
include_directories(${HTTPCLIENT_PATH})

set(HTTPCLIENT_SOURCES)
get_cxx_files(${HTTPCLIENT_PATH}/httpclient SRC_LIST)
list(APPEND HTTPCLIENT_SOURCES ${SRC_LIST})
source_group(httpclient FILES ${HTTPCLIENT_SOURCES})

# 整合依赖库文件
set(DEPEND_FILES)
list(APPEND DEPEND_FILES ${THREADING_TASK_SOURCES} ${THREADING_ASIO_SOURCES})
if(ENABLE_FIBER)
    list(APPEND DEPEND_FILES ${THREADING_FIBER_SOURCES})
endif()
list(APPEND DEPEND_FILES ${THREADING_DIAGNOSE_SOURCES} ${THREADING_SOURCES})
list(APPEND DEPEND_FILES ${CURLEX_SOURCES})
list(APPEND DEPEND_FILES ${HTTPCLIENT_SOURCES})
print_info(BODY "depend files:")
foreach(filename ${DEPEND_FILES})
    message("    " ${filename})
endforeach()

# 示例文件
set(EXAMPLE_FILES)
get_cxx_files(. SRC_LIST)
list(APPEND EXAMPLE_FILES ${MODULE_FILES} ${SRC_LIST})
print_info(BODY "example files:")
foreach(filename ${EXAMPLE_FILES})
    message("    " ${filename})
endforeach()

add_executable(example_httpclient ${DEPEND_FILES} ${EXAMPLE_FILES})
target_link_libraries(example_httpclient ${Boost_LIBRARIES} Threads::Threads CURL::libcurl)
