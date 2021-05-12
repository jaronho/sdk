cmake_minimum_required(VERSION 3.18.0)

project(example_curlex)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

find_package(CURL 7.76.1 REQUIRED)

if(CURL_FOUND)
    message(STATUS "CURL include path: ${CURL_INCLUDE_DIRS}")
    include_directories(${CURL_INCLUDE_DIRS})
else()
    message(WARNING "CURL not found")
endif()

set(CURLEX_SOURCES)
get_cxx_files(../curlex SRC_LIST)
list(APPEND CURLEX_SOURCES ${SRC_LIST})
source_group(curlex FILES ${CURLEX_SOURCES})

set(DEPEND_FILES)
list(APPEND DEPEND_FILES ${CURLEX_SOURCES})
print_info(BODY "depend files:")
foreach(filename ${DEPEND_FILES})
    message("    " ${filename})
endforeach()

set(EXAMPLE_FILES)
get_cxx_files(. SRC_LIST)
list(APPEND EXAMPLE_FILES ${SRC_LIST})
print_info(BODY "example files:")
foreach(filename ${EXAMPLE_FILES})
    message("    " ${filename})
endforeach()

add_executable(example_curlex ${DEPEND_FILES} ${EXAMPLE_FILES})
target_link_libraries(example_curlex CURL::libcurl)
