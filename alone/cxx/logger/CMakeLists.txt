cmake_minimum_required(VERSION 3.18.0)

project(logger)

include(cmake/tools.cmake)

include_directories(fmt-7.1.3/include)

set(LOGGER_FILES)
get_cxx_files(fmt-7.1.3/src SRC_LIST)
list(APPEND LOGGER_FILES ${SRC_LIST})
get_cxx_files(logger SRC_LIST)
list(APPEND LOGGER_FILES ${SRC_LIST})

print_info(BODY "logger file list:")
foreach(filename ${LOGGER_FILES})
    message("    " ${filename})
endforeach()
