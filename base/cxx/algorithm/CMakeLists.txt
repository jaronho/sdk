cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14)

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源文件]
${base_algorithm_files}
]]

##################################### 添加algorithm库 ####################################
set(base_algorithm_base64_src_list)
get_cxx_files(algorithm/base64 src_list)
list(APPEND base_algorithm_base64_src_list ${src_list})

set(base_algorithm_md5_src_list)
get_cxx_files(algorithm/md5 src_list)
list(APPEND base_algorithm_md5_src_list ${src_list})

set(base_algorithm_rc4_src_list)
get_cxx_files(algorithm/rc4 src_list)
list(APPEND base_algorithm_rc4_src_list ${src_list})

set(base_algorithm_sha1_src_list)
get_cxx_files(algorithm/sha1 src_list)
list(APPEND base_algorithm_sha1_src_list ${src_list})

set(base_algorithm_sm3_src_list)
get_cxx_files(algorithm/sm3 src_list)
list(APPEND base_algorithm_sm3_src_list ${src_list})

set(base_algorithm_sm4_src_list)
get_cxx_files(algorithm/sm4 src_list)
list(APPEND base_algorithm_sm4_src_list ${src_list})

set(base_algorithm_snowflake_src_list)
get_cxx_files(algorithm/snowflake src_list)
list(APPEND base_algorithm_snowflake_src_list ${src_list})

set(base_algorithm_src_list)
get_cxx_files(algorithm src_list)
list(APPEND base_algorithm_src_list ${src_list})

# 组装文件列表
set(base_algorithm_files
    ${base_algorithm_base64_src_list}
    ${base_algorithm_md5_src_list}
    ${base_algorithm_rc4_src_list}
    ${base_algorithm_sha1_src_list}
    ${base_algorithm_sm3_src_list}
    ${base_algorithm_sm4_src_list}
    ${base_algorithm_snowflake_src_list}
    ${base_algorithm_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base algorithm files:")
foreach(filename ${base_algorithm_files})
    message("    " ${filename})
endforeach()
