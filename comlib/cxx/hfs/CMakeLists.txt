cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 输出 ]#########################################
#[[
[源目录]
${comlib_hfs_dir}

[源文件]
${comlib_hfs_files}
]]

#################################### 添加hfs模块 ###################################
set(comlib_hfs_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

# 组装文件列表
set(comlib_hfs_files
    ${CMAKE_CURRENT_SOURCE_DIR}/hfs/hfs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hfs/hfs.h
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "comlib hfs files:")
foreach(filename ${comlib_hfs_files})
    message("    " ${filename})
endforeach()
