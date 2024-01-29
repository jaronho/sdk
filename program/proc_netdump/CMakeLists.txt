####################################### 添加PCAP库 #######################################
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_path(PCAP_INCLUDE_DIR NAMES pcap.h)
    find_library(PCAP_LIBRARIES NAMES wpcap HINTS ${PCAP_INCLUDE_DIR}/../Lib/x64) # 如果工程要生成32位的, 则不需要加x64
else ()
    if ("$ENV{PCAP_ROOT}" STREQUAL "") # 自动查找
        find_path(PCAP_INCLUDE_DIR NAMES pcap.h)
    else () # 如果有手动配置PCAP环境变量PCAP_ROOT, 则从环境变量中获取
        set(PCAP_INCLUDE_DIR $ENV{PCAP_ROOT}/include)
    endif ()
    find_library(PCAP_LIBRARIES NAMES pcap HINTS ${PCAP_INCLUDE_DIR}/../lib)
endif ()
set(PCAP_ROOT_DIR ${PCAP_INCLUDE_DIR}/../)
message(STATUS "Pcap root dir: ${PCAP_ROOT_DIR}")
message(STATUS "Pcap include path: ${PCAP_INCLUDE_DIR}")
message(STATUS "Pcap libraries: ${PCAP_LIBRARIES}")
if ("${PCAP_ROOT_DIR}" STREQUAL "PCAP_ROOT_DIR-NOTFOUND" OR
        "${PCAP_INCLUDE_DIR}" STREQUAL "PCAP_INCLUDE_DIR-NOTFOUND" OR
        "${PCAP_LIBRARIES}" STREQUAL "PCAP_LIBRARIES-NOTFOUND")
    message(WARNING "Pcap not found")
    return()
else ()
    include_directories(${PCAP_INCLUDE_DIR})
endif ()
##########################################################################################

######################################## 添加基础库 #######################################
# 添加npacket基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../base/cxx/npacket 3rdparty/base/npacket)
include_directories(${base_npacket_dir})
source_group(TREE ${base_npacket_dir} PREFIX 3rdparty/base FILES ${base_npacket_files})

# 添加utility基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../base/cxx/utility 3rdparty/base/utility)
include_directories(${base_utility_dir})
source_group(TREE ${base_utility_dir} PREFIX 3rdparty/base FILES ${base_utility_files})
##########################################################################################

# 添加工程文件
set(proc_files)
get_cxx_files(. src_list)
list(APPEND proc_files ${src_list})

if (MSVC)
    add_compile_options("/utf-8") # 添加UTF8编码支持
endif()

# 构建可执行文件
add_executable(netdump
               ${base_npacket_files}
               ${base_utility_files}
               ${proc_files})

# 链接依赖库文件
target_link_libraries(netdump
                      Threads::Threads
                      ${PCAP_LIBRARIES})
