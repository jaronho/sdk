######################################## 添加基础库 #######################################
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
add_executable(textfile2codestring
               ${base_utility_files}
               ${proc_files})

# 链接依赖库文件
target_link_libraries(textfile2codestring
                      Threads::Threads)
