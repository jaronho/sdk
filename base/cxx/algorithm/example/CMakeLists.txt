cmake_minimum_required(VERSION 3.18.0)

project(example_algorithm)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加algorithm库 ###################################
set(base_algorithm_path ..) # 设置algorithm库路径(可修改)
include_directories(${base_algorithm_path})

set(base_algorithm_base64_src_list)
get_cxx_files(${base_algorithm_path}/algorithm/base64 src_list)
list(APPEND base_algorithm_base64_src_list ${src_list})
source_group(3rdparty/base/algorithm/base64 FILES ${base_algorithm_base64_src_list})

set(base_algorithm_md5_src_list)
get_cxx_files(${base_algorithm_path}/algorithm/md5 src_list)
list(APPEND base_algorithm_md5_src_list ${src_list})
source_group(3rdparty/base/algorithm/md5 FILES ${base_algorithm_md5_src_list})

set(base_algorithm_rc4_src_list)
get_cxx_files(${base_algorithm_path}/algorithm/rc4 src_list)
list(APPEND base_algorithm_rc4_src_list ${src_list})
source_group(3rdparty/base/algorithm/rc4 FILES ${base_algorithm_rc4_src_list})

set(base_algorithm_sha1_src_list)
get_cxx_files(${base_algorithm_path}/algorithm/sha1 src_list)
list(APPEND base_algorithm_sha1_src_list ${src_list})
source_group(3rdparty/base/algorithm/sha1 FILES ${base_algorithm_sha1_src_list})

set(base_algorithm_sm3_src_list)
get_cxx_files(${base_algorithm_path}/algorithm/sm3 src_list)
list(APPEND base_algorithm_sm3_src_list ${src_list})
source_group(3rdparty/base/algorithm/sm3 FILES ${base_algorithm_sm3_src_list})

set(base_algorithm_sm4_src_list)
get_cxx_files(${base_algorithm_path}/algorithm/sm4 src_list)
list(APPEND base_algorithm_sm4_src_list ${src_list})
source_group(3rdparty/base/algorithm/sm4 FILES ${base_algorithm_sm4_src_list})

set(base_algorithm_snowflake_src_list)
get_cxx_files(${base_algorithm_path}/algorithm/snowflake src_list)
list(APPEND base_algorithm_snowflake_src_list ${src_list})
source_group(3rdparty/base/algorithm/snowflake FILES ${base_algorithm_snowflake_src_list})

set(base_algorithm_src_list)
get_cxx_files(${base_algorithm_path}/algorithm src_list)
list(APPEND base_algorithm_src_list ${src_list})
source_group(3rdparty/base/algorithm FILES ${base_algorithm_src_list})

set(base_algorithm_files) # 工程中需要把algorithm的文件添加进去
list(APPEND base_algorithm_files ${base_algorithm_base64_src_list}
                                 ${base_algorithm_md5_src_list}
                                 ${base_algorithm_rc4_src_list}
                                 ${base_algorithm_sha1_src_list}
                                 ${base_algorithm_sm3_src_list}
                                 ${base_algorithm_sm4_src_list}
                                 ${base_algorithm_snowflake_src_list}
                                 ${base_algorithm_src_list})

print_info(BODY "base algorithm files:")
foreach(filename ${base_algorithm_files})
    message("    " ${filename})
endforeach()
##########################################################################################

# 添加示例文件
set(example_files)
get_cxx_files(. src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_algorithm ${base_algorithm_files} ${example_files})

# 链接依赖库
target_link_libraries(example_algorithm)
