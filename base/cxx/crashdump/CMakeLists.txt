cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14)

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[命令行]
find_package(Threads REQUIRED)

[链接库]
Threads::Threads
]]

#########################################[ 输出 ]#########################################
#[[
[链接库]
${breakpad_libraries}

[源目录]
${base_crashdump_dir}

[源文件]
${base_crashdump_files}
]]

##################################### 添加breakpad库 #####################################
if (CMAKE_SYSTEM_NAME MATCHES "Windows") # 注意: Windows平台需要在VS工程中手动指定`头文件搜索路径`和`库文件搜索路径`
    set(breakpad_libraries common crash_generation_client exception_handler CACHE INTERNAL "")
else()
    set(breakpad_libraries breakpad_client CACHE INTERNAL "")
endif()

##################################### 添加crashdump库 ####################################
set(base_crashdump_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(base_crashdump_src_list)
get_cxx_files(crashdump src_list)
list(APPEND base_crashdump_src_list ${src_list})

# 组装文件列表
set(base_crashdump_files
    ${base_crashdump_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base crashdump files:")
foreach(filename ${base_crashdump_files})
    message("    " ${filename})
endforeach()
