cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14)

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源目录]
${base_fileparse_dir}

[源文件]
${base_fileparse_files}
]]

##################################### 添加fileparse库 ####################################
set(base_fileparse_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(base_fileparse_ini_src_list)
get_cxx_files(fileparse/ini src_list)
list(APPEND base_fileparse_ini_src_list ${src_list})

set(base_fileparse_nlohmann_src_list)
get_cxx_files(fileparse/nlohmann src_list)
list(APPEND base_fileparse_nlohmann_src_list ${src_list})

# 组装文件列表
set(base_fileparse_files
    ${base_fileparse_ini_src_list}
    ${base_fileparse_nlohmann_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base fileparse files:")
foreach(filename ${base_fileparse_files})
    message("    " ${filename})
endforeach()
