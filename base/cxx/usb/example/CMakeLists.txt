cmake_minimum_required(VERSION 3.18.0)

project(example_usb)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(../cmake/tools.cmake)

####################################### 添加线程库 #######################################
find_package(Threads REQUIRED)
##########################################################################################

# 添加usb基础库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. 3rdparty/base/usb)
include_directories(${libusb_include_dir})
include_directories(${libusb_platform_include_dir})
include_directories(${base_usb_dir})
source_group(TREE ${base_usb_dir} PREFIX 3rdparty/base FILES ${base_usb_files})

# 构建可执行程序
add_executable(example_usb ${base_usb_files} example_usb.cpp)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_executable(example_win_detect ${base_usb_files} example_win_detect.cpp)
    add_executable(example_win_usb ${base_usb_files} example_win_usb.cpp)
endif()
add_executable(usb_watch ${base_usb_files} usb_watch.cpp)

# 链接依赖库文件
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(example_usb Threads::Threads udev)
    target_link_libraries(usb_watch Threads::Threads udev)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(example_usb Threads::Threads)
    target_link_libraries(example_win_detect Threads::Threads)
    target_link_libraries(example_win_usb Threads::Threads)
    target_link_libraries(usb_watch Threads::Threads)
endif()
