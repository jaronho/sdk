cmake_minimum_required(VERSION 3.18.0)

project(example_utility)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

# 添加utility基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/utility)
include_directories(${base_utility_dir})
source_group(TREE ${base_utility_dir} PREFIX 3rdparty/base FILES ${base_utility_files})

if (MSVC)
    add_compile_options("/utf-8") # 添加UTF8编码支持
endif()

# 构建可执行程序
add_executable(example_filecopy ${base_utility_files} example_filecopy.cpp)
add_executable(example_fileinfo ${base_utility_files} example_fileinfo.cpp)
add_executable(example_utility ${base_utility_files} module_a.cpp module_a.h
                                                      module_b.cpp module_b.h
                                                      module_c.h module_c_impl.cpp module_c_impl.h
                                                      test_bytearray.hpp
                                                      test_charset.hpp
                                                      test_datetime.hpp
                                                      test_filesystem.hpp
                                                      test_mmfile.hpp
                                                      test_module.hpp
                                                      test_net.hpp
                                                      test_process.hpp
                                                      test_system.hpp
                                                      test_timewatch.hpp
                                                      test_util.hpp
                                                      example_utility.cpp)

# 链接依赖库
target_link_libraries(example_filecopy Threads::Threads)
target_link_libraries(example_fileinfo Threads::Threads)
target_link_libraries(example_utility Threads::Threads)
