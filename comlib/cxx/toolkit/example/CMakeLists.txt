cmake_minimum_required(VERSION 3.18.0)

project(example_toolkit)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

# 添加algorithm基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/algorithm 3rdparty/base/algorithm)
include_directories(${base_algorithm_dir})
source_group(TREE ${base_algorithm_dir} PREFIX 3rdparty/base FILES ${base_algorithm_files})

# 添加fileparse基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/fileparse 3rdparty/base/fileparse)
include_directories(${base_fileparse_dir})
source_group(TREE ${base_fileparse_dir} PREFIX 3rdparty/base FILES ${base_fileparse_files})

# 添加utility基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../base/cxx/utility 3rdparty/base/utility)
include_directories(${base_utility_dir})
source_group(TREE ${base_utility_dir} PREFIX 3rdparty/base FILES ${base_utility_files})

# 添加toolkit公共库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/comlib/toolkit)
include_directories(${comlib_toolkit_dir})
source_group(TREE ${comlib_toolkit_dir} PREFIX 3rdparty/comlib FILES ${comlib_toolkit_files})

if (MSVC)
    add_compile_options("/utf-8") # 添加UTF8编码支持
endif()

# 构建可执行程序
add_executable(example_app_singleton
			   ${base_utility_filesystem_files}
			   ${base_utility_process_files}
			   ${base_utility_system_files}
			   ${comlib_toolkit_app_singleton_files}
			   example_app_singleton.cpp)

add_executable(example_toolkit
			   ${base_utility_filesystem_files}
			   ${base_utility_net_files}
			   ${base_utility_strtool_files}
			   ${base_utility_system_files}
			   ${comlib_toolkit_net_config_files}
			   example_toolkit.cpp)

add_executable(file_deleter_tool
               ${base_fileparse_files}
			   ${base_utility_cmdline_files}
			   ${base_utility_datetime_files}
			   ${base_utility_filesystem_files}
			   ${base_utility_strtool_files}
			   ${base_utility_system_files}
			   ${comlib_toolkit_file_deleter_files}
			   file_deleter_tool.cpp)

# 链接依赖库
target_link_libraries(example_app_singleton Threads::Threads)
target_link_libraries(example_toolkit Threads::Threads)
target_link_libraries(file_deleter_tool Threads::Threads)
