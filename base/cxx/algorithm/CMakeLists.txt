cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源目录]
${base_algorithm_dir}

[源文件]
${base_algorithm_files}

[源文件]
${base_algorithm_atomic_files}

[源文件]
${base_algorithm_base64_files}

[源文件]
${base_algorithm_md5_files}

[源文件]
${base_algorithm_rc4_files}

[源文件]
${base_algorithm_sha1_files}

[源文件]
${base_algorithm_sm3_files}

[源文件]
${base_algorithm_sm4_files}

[源文件]
${base_algorithm_snowflake_files}

[源文件]
${base_algorithm_tvalue_files}
]]

##################################### 添加algorithm库 ####################################
set(base_algorithm_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

get_cxx_files(algorithm/atomic src_list)
set(base_algorithm_atomic_files ${src_list} CACHE INTERNAL "")

get_cxx_files(algorithm/base64 src_list)
set(base_algorithm_base64_files ${src_list} CACHE INTERNAL "")

get_cxx_files(algorithm/md5 src_list)
set(base_algorithm_md5_files ${src_list} CACHE INTERNAL "")

get_cxx_files(algorithm/rc4 src_list)
set(base_algorithm_rc4_files ${src_list} CACHE INTERNAL "")

get_cxx_files(algorithm/sha1 src_list)
set(base_algorithm_sha1_files ${src_list} CACHE INTERNAL "")

get_cxx_files(algorithm/sm3 src_list)
set(base_algorithm_sm3_files ${src_list} CACHE INTERNAL "")

get_cxx_files(algorithm/sm4 src_list)
set(base_algorithm_sm4_files ${src_list} CACHE INTERNAL "")

get_cxx_files(algorithm/snowflake src_list)
set(base_algorithm_snowflake_files ${src_list} CACHE INTERNAL "")

get_cxx_files(algorithm/tvalue src_list)
set(base_algorithm_tvalue_files ${src_list} CACHE INTERNAL "")

# 组装文件列表
set(base_algorithm_files
    ${base_algorithm_atomic_files}
    ${base_algorithm_base64_files}
    ${base_algorithm_md5_files}
    ${base_algorithm_rc4_files}
    ${base_algorithm_sha1_files}
    ${base_algorithm_sm3_files}
    ${base_algorithm_sm4_files}
    ${base_algorithm_snowflake_files}
    ${base_algorithm_tvalue_files}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base algorithm files:")
foreach(filename ${base_algorithm_files})
    message("    " ${filename})
endforeach()
