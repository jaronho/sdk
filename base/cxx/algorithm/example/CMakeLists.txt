cmake_minimum_required(VERSION 3.18.0)

project(example_algorithm)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加algorithm库 ###################################
set(algorithm_path ..) # 设置algorithm库路径(可修改)
include_directories(${algorithm_path})

set(algorithm_base64_src_list)
get_cxx_files(${algorithm_path}/algorithm/base64 src_list)
list(APPEND algorithm_base64_src_list ${src_list})
source_group(3rdparty/algorithm/base64 FILES ${algorithm_base64_src_list})

set(algorithm_md5_src_list)
get_cxx_files(${algorithm_path}/algorithm/md5 src_list)
list(APPEND algorithm_md5_src_list ${src_list})
source_group(3rdparty/algorithm/md5 FILES ${algorithm_md5_src_list})

set(algorithm_rc4_src_list)
get_cxx_files(${algorithm_path}/algorithm/rc4 src_list)
list(APPEND algorithm_rc4_src_list ${src_list})
source_group(3rdparty/algorithm/rc4 FILES ${algorithm_rc4_src_list})

set(algorithm_sm3_src_list)
get_cxx_files(${algorithm_path}/algorithm/sm3 src_list)
list(APPEND algorithm_sm3_src_list ${src_list})
source_group(3rdparty/algorithm/sm3 FILES ${algorithm_sm3_src_list})

set(algorithm_snowflake_src_list)
get_cxx_files(${algorithm_path}/algorithm/snowflake src_list)
list(APPEND algorithm_snowflake_src_list ${src_list})
source_group(3rdparty/algorithm/snowflake FILES ${algorithm_snowflake_src_list})

set(algorithm_src_list)
get_cxx_files(${algorithm_path}/algorithm src_list)
list(APPEND algorithm_src_list ${src_list})
source_group(3rdparty/algorithm FILES ${algorithm_src_list})

set(algorithm_files) # 工程中需要把algorithm的文件添加进去
list(APPEND algorithm_files ${algorithm_base64_src_list}
                            ${algorithm_md5_src_list}
                            ${algorithm_rc4_src_list}
                            ${algorithm_sm3_src_list}
                            ${algorithm_snowflake_src_list}
                            ${algorithm_src_list})

print_info(BODY "algorithm files:")
foreach(filename ${algorithm_files})
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
add_executable(example_algorithm ${algorithm_files} ${example_files})

# 链接依赖库
target_link_libraries(example_algorithm)
