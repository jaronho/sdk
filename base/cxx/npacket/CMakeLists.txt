cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14)

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[命令行]
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_path(PCAP_ROOT_DIR NAMES Include/pcap.h )
    find_path(PCAP_INCLUDE_DIR NAMES pcap.h HINTS ${PCAP_ROOT_DIR}/Include)
    find_library(PCAP_LIBRARIES NAMES wpcap HINTS ${PCAP_ROOT_DIR}/Lib/x64) # 如果工程要生成32位的, 则不需要加x64
else()
    find_path(PCAP_ROOT_DIR NAMES include/pcap.h )
    find_path(PCAP_INCLUDE_DIR NAMES pcap.h HINTS ${PCAP_ROOT_DIR}/include)
    find_library(PCAP_LIBRARIES NAMES pcap HINTS ${PCAP_ROOT_DIR}/lib)
endif()
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

[链接库]
${PCAP_LIBRARIES}
]]

#########################################[ 输出 ]#########################################
#[[
[源目录]
${base_npacket_dir}

[源文件]
${base_npacket_files}
]]

###################################### 添加npacket库 #####################################
set(base_npacket_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(base_npacket_src_list)
get_cxx_files(npacket src_list)

# 组装文件列表
set(base_npacket_files
    ${base_npacket_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base npacket files:")
foreach(filename ${base_npacket_files})
    message("    " ${filename})
endforeach()
