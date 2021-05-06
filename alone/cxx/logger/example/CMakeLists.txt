cmake_minimum_required(VERSION 3.18.0)

project(example_logger)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

option(ENABLE_DETAIL "" ON)

if(ENABLE_DETAIL)
    add_definitions(-DENABLE_LOGGER_DETAIL=1)
else()
    add_definitions(-DENABLE_LOGGER_DETAIL=0)
endif()

include_directories(../fmt-7.1.3/include)
set(FMT_SOURCES)
get_cxx_files(../fmt-7.1.3/src SRC_LIST)
list(APPEND FMT_SOURCES ${SRC_LIST})
source_group(fmt FILES ${FMT_SOURCES})

set(LOGGER_LOGFILE_SOURCES)
get_cxx_files(../logger/logfile SRC_LIST)
list(APPEND LOGGER_LOGFILE_SOURCES ${SRC_LIST})
source_group(logger/logfile FILES ${LOGGER_LOGFILE_SOURCES})

set(LOGGER_IMPL_SOURCES)
get_cxx_files(../logger/impl SRC_LIST)
list(APPEND LOGGER_IMPL_SOURCES ${SRC_LIST})
source_group(logger/impl FILES ${LOGGER_IMPL_SOURCES})

set(LOGGER_SOURCES)
get_cxx_files(../logger SRC_LIST)
list(APPEND LOGGER_SOURCES ${SRC_LIST})
source_group(logger FILES ${LOGGER_SOURCES})

set(DEPEND_FILES)
list(APPEND DEPEND_FILES ${FMT_SOURCES} ${LOGGER_LOGFILE_SOURCES} ${LOGGER_IMPL_SOURCES} ${LOGGER_SOURCES})
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

add_executable(example_logger ${DEPEND_FILES} ${EXAMPLE_FILES})
