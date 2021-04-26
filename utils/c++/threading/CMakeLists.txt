cmake_minimum_required(VERSION 3.18.0)

project(threading)

include(cmake/tools.cmake)

option(ENABLE_FIBER "" OFF)

if (ENABLE_FIBER)
    find_package(Boost 1.76.0 COMPONENTS fiber REQUIRED)
    add_definitions(ENABLE_THREADING_FIBER=1)
else()
    find_package(Boost 1.76.0 COMPONENTS REQUIRED)
    add_definitions(ENABLE_THREADING_FIBER=0)
endif()

set(THREADING_FILES)
get_cxx_files(threading/asio SRC_LIST)
list(APPEND THREADING_FILES ${SRC_LIST})
get_cxx_files(threading/diagnose SRC_LIST)
list(APPEND THREADING_FILES ${SRC_LIST})
if (ENABLE_FIBER)
    get_cxx_files(threading/fiber SRC_LIST)
    list(APPEND THREADING_FILES ${SRC_LIST})
endif()
get_cxx_files(threading/task SRC_LIST)
list(APPEND THREADING_FILES ${SRC_LIST})
get_cxx_files(threading SRC_LIST)
list(APPEND THREADING_FILES ${SRC_LIST})

print_info(BODY "threading file list:")
foreach(filename ${THREADING_FILES})
    message("    " ${filename})
endforeach()
