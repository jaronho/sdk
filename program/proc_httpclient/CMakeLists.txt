######################################## 添加基础库 #######################################
# 添加curlex基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../base/cxx/curlex 3rdparty/base/curlex)
include_directories(${base_curlex_dir})
source_group(TREE ${base_curlex_dir} PREFIX 3rdparty/base FILES ${base_curlex_files})

# 添加threading基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../base/cxx/threading 3rdparty/base/threading)
include_directories(${base_threading_dir})
source_group(TREE ${base_threading_dir} PREFIX 3rdparty/base FILES ${base_threading_files})

# 添加utility基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../base/cxx/utility 3rdparty/base/utility)
include_directories(${base_utility_dir})
source_group(TREE ${base_utility_dir} PREFIX 3rdparty/base FILES ${base_utility_files})

# 添加httpclient公共库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../comlib/cxx/httpclient 3rdparty/comlib/httpclient)
include_directories(${comlib_httpclient_dir})
source_group(TREE ${comlib_httpclient_dir} PREFIX 3rdparty/comlib FILES ${comlib_httpclient_files})
##########################################################################################

# 添加工程文件
set(proc_files)
get_cxx_files(. src_list)
list(APPEND proc_files ${src_list})

if (MSVC)
    add_compile_options("/utf-8") # 添加UTF8编码支持
endif()

# 构建可执行文件
add_executable(httpclient
               ${base_curlex_files}
               ${base_threading_files}
               ${base_utility_files}
               ${comlib_httpclient_files}
               ${proc_files})

# 链接依赖库文件
target_link_libraries(httpclient
                      Threads::Threads
                      ${curl_libraries}
                      ${Boost_LIBRARIES})

# 拷贝动态链接库
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_custom_command(
        TARGET httpclient
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CURL_INCLUDE_DIRS}/../bin/libcurl.dll "$(OutDir)/libcurl.dll"
        COMMAND ${CMAKE_COMMAND} -E copy ${CURL_INCLUDE_DIRS}/../bin/libcurl-d.dll "$(OutDir)/libcurl-d.dll"
    )
endif()
