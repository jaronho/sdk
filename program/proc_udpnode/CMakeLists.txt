######################################## 添加基础库 #######################################
# 添加nsocket基础库
option(enable_nsocket_openssl "是否启用套接字OpenSSL依赖" ON)
if (enable_nsocket_openssl)
    add_definitions(-DENABLE_NSOCKET_OPENSSL=1)
else ()
    add_definitions(-DENABLE_NSOCKET_OPENSSL=0)
endif ()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../base/cxx/nsocket 3rdparty/base/nsocket)
include_directories(${base_nsocket_dir})
source_group(TREE ${base_nsocket_dir} PREFIX 3rdparty/base FILES ${base_nsocket_files})

# 添加utility基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../base/cxx/utility 3rdparty/base/utility)
include_directories(${base_utility_dir})
source_group(TREE ${base_utility_dir} PREFIX 3rdparty/base FILES ${base_utility_files})
##########################################################################################

# 添加工程文件
set(proc_files)
get_cxx_files(. src_list)
list(APPEND proc_files ${src_list})

# 构建可执行文件
add_executable(udpnode
               ${base_nsocket_udp_files}
               ${base_utility_files}
               ${proc_files})

# 链接依赖库文件
target_link_libraries(udpnode
                      ${OPENSSL_LIBRARIES})
