cmake_minimum_required(VERSION 3.18.0)

project(module)

include(cmake/tools.cmake)

set(MODULE_FILES)
get_cxx_files(module SRC_LIST)
list(APPEND MODULE_FILES ${SRC_LIST})

print_info(BODY "module file list:")
foreach(filename ${MODULE_FILES})
    message("    " ${filename})
endforeach()
