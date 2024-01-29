cmake_minimum_required(VERSION 3.18.0)

project(example_ipc)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

# 添加ipc基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/ipc)
include_directories(${base_ipc_dir})
source_group(TREE ${base_ipc_dir} PREFIX 3rdparty/base FILES ${base_ipc_files})

# 添加simdb接收示例文件
set(example_simdb_recv_files)
list(APPEND example_simdb_recv_files example_simdb_recv.cpp)

print_info(BODY "example simdb recv files:")
foreach(filename ${example_simdb_recv_files})
    message("    " ${filename})
endforeach()

# 添加simdb发送示例文件
set(example_simdb_send_files)
list(APPEND example_simdb_send_files example_simdb_send.cpp)

print_info(BODY "example simdb send files:")
foreach(filename ${example_simdb_send_files})
    message("    " ${filename})
endforeach()

if (MSVC)
    add_compile_options("/utf-8") # 添加UTF8编码支持
endif()

# 构建可执行程序
add_executable(example_simdb_recv ${base_ipc_simdb_files} ${example_simdb_recv_files})
add_executable(example_simdb_send ${base_ipc_simdb_files} ${example_simdb_send_files})

# 链接依赖库
target_link_libraries(example_simdb_recv)
target_link_libraries(example_simdb_send)
