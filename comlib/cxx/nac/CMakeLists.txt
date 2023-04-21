cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源目录]
${comlib_nac_dir}

[源文件]
${comlib_nac_tclient_files}

[源文件]
${comlib_nac_wrapper_files}

[源文件]
${comlib_nac_files}
]]

#################################### 添加nac模块 ###################################
set(comlib_nac_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(comlib_nac_tclient_impl_src_list)
get_cxx_files(nac/tclient/impl src_list)
list(APPEND comlib_nac_tclient_impl_src_list ${src_list})

set(comlib_nac_tclient_src_list)
get_cxx_files(nac/tclient src_list)
list(APPEND comlib_nac_tclient_src_list ${src_list})

set(comlib_nac_wrapper_src_list)
get_cxx_files(nac/wrapper src_list)
list(APPEND comlib_nac_wrapper_src_list ${src_list})

set(comlib_nac_src_list)
get_cxx_files(nac src_list)
list(APPEND comlib_nac_src_list ${src_list})

# 组装文件列表
set(comlib_nac_tclient_files
    ${comlib_nac_tclient_impl_src_list}
    ${comlib_nac_tclient_src_list}
    ${comlib_nac_wrapper_src_list}
    ${comlib_nac_src_list}
    CACHE INTERNAL "")

set(comlib_nac_wrapper_files
    ${comlib_nac_wrapper_src_list}
    ${comlib_nac_src_list}
    CACHE INTERNAL "")
    
set(comlib_nac_files
    ${comlib_nac_tclient_impl_src_list}
    ${comlib_nac_tclient_src_list}
    ${comlib_nac_wrapper_src_list}
    ${comlib_nac_src_list}
    CACHE INTERNAL "")

# 打印文件列表
if (ENABLE_FILES_INFO)
    print_info(BODY "comlib nac files:")
    foreach(filename ${comlib_nac_files})
        message("    " ${filename})
    endforeach()
endif()