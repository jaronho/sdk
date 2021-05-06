cmake_minimum_required(VERSION 3.18.0)

project(example_module)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

set(MODULE_FILES)
get_cxx_files(../module SRC_LIST)
list(APPEND MODULE_FILES ${SRC_LIST})
print_info(BODY "module file list:")
foreach(filename ${MODULE_FILES})
    message("    " ${filename})
endforeach()

set(EXAMPLE_FILES)
get_cxx_files(. SRC_LIST)
list(APPEND EXAMPLE_FILES ${MODULE_FILES} ${SRC_LIST})
print_info(BODY "module file list:")
foreach(filename ${EXAMPLE_FILES})
    message("    " ${filename})
endforeach()

add_executable(example_module ${MODULE_FILES} ${EXAMPLE_FILES})
