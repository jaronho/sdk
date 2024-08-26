######################################## 添加基础库 #######################################
# 添加logger基础库
option(enable_logger_detail "是否启用日志详情(包含文件名,行号,函数名)" ON)
if (enable_logger_detail)
    add_definitions(-DENABLE_LOGGER_DETAIL=2)
else ()
    add_definitions(-DENABLE_LOGGER_DETAIL=0)
endif ()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../base/cxx/logger 3rdparty/base/logger)
include_directories(${fmt_include_dir})
include_directories(${base_logger_dir})
source_group(TREE ${base_logger_dir} PREFIX 3rdparty/base FILES ${base_logger_files})

# 添加utility基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../base/cxx/utility 3rdparty/base/utility)
include_directories(${base_utility_dir})
source_group(TREE ${base_utility_dir} PREFIX 3rdparty/base FILES ${base_utility_files})
##########################################################################################

# 添加toolkit公共库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../comlib/cxx/toolkit/ 3rdparty/comlib/toolkit)
include_directories(${comlib_toolkit_dir})
source_group(TREE ${comlib_toolkit_dir} PREFIX 3rdparty/comlib FILES ${comlib_toolkit_net_config_files})

# 添加工程文件
set(proc_files)
get_cxx_files(. src_list)
list(APPEND proc_files ${src_list})

if (MSVC)
    add_compile_options("/utf-8") # 添加UTF8编码支持
endif()

# 构建可执行文件
add_executable(hping
               ${base_logger_files}
               ${base_utility_files}
               ${comlib_toolkit_net_config_files}
               ${proc_files})

# 链接依赖库文件
target_link_libraries(hping
                      Threads::Threads)
