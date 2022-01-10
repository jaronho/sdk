cmake_minimum_required(VERSION 3.18.0)

project(example_usb)

set(CMAKE_CXX_STANDARD 14)

include(../cmake/tools.cmake)

################################### 添加usb库 ###################################
set(base_usb_path ..) # 设置usb库路径(可修改)
include_directories(${base_usb_path})

set(base_libusb_src_list)
get_cxx_files(${base_usb_path}/libusb-1.0.24 src_list)
list(APPEND base_libusb_src_list ${src_list})
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    list(APPEND base_libusb_src_list ${base_usb_path}/libusb-1.0.24/oc/events_posix.c
                                     ${base_usb_path}/libusb-1.0.24/oc/events_posix.h
                                     ${base_usb_path}/libusb-1.0.24/oc/linux_netlink.c
                                     ${base_usb_path}/libusb-1.0.24/oc/linux_udev.c
                                     ${base_usb_path}/libusb-1.0.24/oc/linux_usbfs.c
                                     ${base_usb_path}/libusb-1.0.24/oc/linux_usbfs.h
                                     ${base_usb_path}/libusb-1.0.24/oc/null_usb.c
                                     ${base_usb_path}/libusb-1.0.24/oc/threads_posix.c
                                     ${base_usb_path}/libusb-1.0.24/oc/threads_posix.h)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows)
    list(APPEND base_libusb_src_list ${base_usb_path}/libusb-1.0.24/oc/events_windows.c
                                     ${base_usb_path}/libusb-1.0.24/oc/events_windows.h
                                     ${base_usb_path}/libusb-1.0.24/oc/null_usb.c
                                     ${base_usb_path}/libusb-1.0.24/oc/threads_windows.c
                                     ${base_usb_path}/libusb-1.0.24/oc/threads_windows.h
                                     ${base_usb_path}/libusb-1.0.24/oc/windows_common.c
                                     ${base_usb_path}/libusb-1.0.24/oc/windows_common.h
                                     ${base_usb_path}/libusb-1.0.24/oc/windows_usbdk.c
                                     ${base_usb_path}/libusb-1.0.24/oc/windows_usbdk.h
                                     ${base_usb_path}/libusb-1.0.24/oc/windows_winusb.c
                                     ${base_usb_path}/libusb-1.0.24/oc/windows_winusb.h)
else()
    message(WARNING "Other Platform: ${CMAKE_SYSTEM_NAME}")
    return()
endif()
source_group(3rdparty/base/libusb FILES ${base_libusb_src_list})

set(base_usb_src_list)
get_cxx_files(${base_usb_path}/usb src_list)
list(APPEND base_usb_src_list ${src_list})
source_group(3rdparty/base/usb FILES ${base_usb_src_list})

set(base_usb_files) # 工程中需要把usb的文件添加进去
list(APPEND base_usb_files ${base_libusb_src_list}
                           ${base_usb_src_list})

print_info(BODY "base usb files:")
foreach(filename ${base_usb_files})
    message("    " ${filename})
endforeach()
##########################################################################################

################################### 添加线程库 ###################################
find_package(Threads REQUIRED)
##########################################################################################

# 添加示例文件
set(example_files)
get_cxx_files(. src_list)
list(APPEND example_files ${src_list})

print_info(BODY "example files:")
foreach(filename ${example_files})
    message("    " ${filename})
endforeach()

# 构建可执行程序
add_executable(example_usb ${base_usb_files} ${example_files})

# 链接依赖库文件
target_link_libraries(example_usb Threads::Threads)
