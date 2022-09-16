cmake_minimum_required(VERSION 3.18.0)

project(example_npacket)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

####################################### 添加PCAP库 #######################################
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_path(PCAP_INCLUDE_DIR NAMES pcap.h)
    find_library(PCAP_LIBRARIES NAMES wpcap HINTS ${PCAP_INCLUDE_DIR}/../Lib/x64) # 如果工程要生成32位的, 则不需要加x64
else ()
    find_path(PCAP_INCLUDE_DIR NAMES pcap.h)
    find_library(PCAP_LIBRARIES NAMES pcap HINTS ${PCAP_INCLUDE_DIR}/../lib)
endif ()
set(PCAP_ROOT_DIR ${PCAP_INCLUDE_DIR}/../)
message(STATUS "Pcap root dir: ${PCAP_ROOT_DIR}")
message(STATUS "Pcap include path: ${PCAP_INCLUDE_DIR}")
message(STATUS "Pcap libraries: ${PCAP_LIBRARIES}")
if("${PCAP_ROOT_DIR}" STREQUAL "PCAP_ROOT_DIR-NOTFOUND" OR 
   "${PCAP_INCLUDE_DIR}" STREQUAL "PCAP_INCLUDE_DIR-NOTFOUND" OR 
   "${PCAP_LIBRARIES}" STREQUAL "PCAP_LIBRARIES-NOTFOUND")
    message(WARNING "Pcap not found")
    return()
else()
    include_directories(${PCAP_INCLUDE_DIR})
endif()
##########################################################################################

# 添加npacket基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/npacket)
include_directories(${base_npacket_dir})
source_group(TREE ${base_npacket_dir} PREFIX 3rdparty/base FILES ${base_npacket_files})

# 添加客户端示例文件
set(example_files)
get_cxx_files(. src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_npacket ${base_npacket_files} ${example_files})

# 链接依赖库
target_link_libraries(example_npacket Threads::Threads ${PCAP_LIBRARIES})
