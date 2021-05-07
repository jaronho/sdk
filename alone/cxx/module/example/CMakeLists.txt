cmake_minimum_required(VERSION 3.18.0)

project(example_module)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

set(MODULE_SOURCES)
get_cxx_files(../module SRC_LIST)
list(APPEND MODULE_SOURCES ${SRC_LIST})
source_group(module FILES ${MODULE_SOURCES})

set(DEPEND_FILES)
list(APPEND DEPEND_FILES ${MODULE_SOURCES})
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

add_executable(example_module ${DEPEND_FILES} ${EXAMPLE_FILES})
