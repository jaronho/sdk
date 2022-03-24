cmake_minimum_required(VERSION 3.18.0)

project(example_logger)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

# 添加logger基础库
option(enable_logger_detail "是否启用日志详情(包含文件名,行号,函数名)" ON)
if(enable_logger_detail)
    add_definitions(-DENABLE_LOGGER_DETAIL=2)
else()
    add_definitions(-DENABLE_LOGGER_DETAIL=0)
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/logger)
include_directories(${fmt_include_dir})
include_directories(${base_logger_dir})
source_group(TREE ${base_logger_dir} PREFIX 3rdparty/base FILES ${base_logger_files})

# 添加示例文件
set(example_files)
get_cxx_files(${CMAKE_CURRENT_SOURCE_DIR} src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_logger ${base_logger_files} ${example_files})
