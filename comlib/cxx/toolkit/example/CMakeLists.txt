cmake_minimum_required(VERSION 3.18.0)

project(example_toolkit)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

# 添加utility基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/algorithm 3rdparty/base/algorithm)
include_directories(${base_algorithm_dir})
source_group(TREE ${base_algorithm_dir} PREFIX 3rdparty/base FILES ${base_algorithm_files})

# 添加utility基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/utility 3rdparty/base/utility)
include_directories(${base_utility_dir})
source_group(TREE ${base_utility_dir} PREFIX 3rdparty/base FILES ${base_utility_files})

# 添加toolkit公共库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/comlib/toolkit)
include_directories(${comlib_toolkit_dir})
source_group(TREE ${comlib_toolkit_dir} PREFIX 3rdparty/comlib FILES ${comlib_toolkit_files})

# 构建可执行程序
add_executable(example_app_singleton 
			   ${base_utility_filesystem_files}
			   ${base_utility_process_files}
			   ${base_utility_system_files}
			   ${comlib_toolkit_files}
			   example_app_singleton.cpp)
add_executable(example_tool ${base_utility_files} ${comlib_toolkit_files} example_tool.cpp)

# 链接依赖库
target_link_libraries(example_app_singleton Threads::Threads)
target_link_libraries(example_tool Threads::Threads)
