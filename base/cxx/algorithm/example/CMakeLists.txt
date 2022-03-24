cmake_minimum_required(VERSION 3.18.0)

project(example_algorithm)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

# 添加algorithm基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/algorithm)
include_directories(${base_algorithm_dir})
source_group(TREE ${base_algorithm_dir} PREFIX 3rdparty/base FILES ${base_algorithm_files})

# 添加示例文件
set(example_files)
get_cxx_files(${CMAKE_CURRENT_SOURCE_DIR} src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_algorithm ${base_algorithm_files} ${example_files})

# 链接依赖库
target_link_libraries(example_algorithm ${base_algorithm})
