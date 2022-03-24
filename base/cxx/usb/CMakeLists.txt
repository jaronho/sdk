cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 14) # `std::make_unique`要求最低C++14

include(cmake/tools.cmake)

#########################################[ 依赖 ]#########################################
#[[
[链接库]
udev # Linux平台
]]

#########################################[ 输出 ]#########################################
#[[
[源目录]
${libusb_include_dir}

[源目录]
${libusb_platform_include_dir}

[源目录]
${base_usb_dir}

[源文件]
${base_usb_files}
]]

################################### 添加usb库 ###################################
set(libusb_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/libusb-1.0.24/libusb CACHE INTERNAL "")

set(base_usb_dir ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

set(base_libusb_src_list)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(libusb_platform_include_dir ${libusb_include_dir}/../posix CACHE INTERNAL "")
    list(APPEND base_libusb_src_list ${libusb_include_dir}/os/events_posix.c
                                     ${libusb_include_dir}/os/events_posix.h
                                     ${libusb_include_dir}/os/linux_netlink.c
                                     ${libusb_include_dir}/os/linux_udev.c
                                     ${libusb_include_dir}/os/linux_usbfs.c
                                     ${libusb_include_dir}/os/linux_usbfs.h
                                     ${libusb_include_dir}/os/threads_posix.c
                                     ${libusb_include_dir}/os/threads_posix.h)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(libusb_platform_include_dir ${libusb_include_dir}/../msvc CACHE INTERNAL "")
    list(APPEND base_libusb_src_list ${libusb_include_dir}/os/events_windows.c
                                     ${libusb_include_dir}/os/events_windows.h
                                     ${libusb_include_dir}/os/threads_windows.c
                                     ${libusb_include_dir}/os/threads_windows.h
                                     ${libusb_include_dir}/os/windows_common.c
                                     ${libusb_include_dir}/os/windows_common.h
                                     ${libusb_include_dir}/os/windows_usbdk.c
                                     ${libusb_include_dir}/os/windows_usbdk.h
                                     ${libusb_include_dir}/os/windows_winusb.c
                                     ${libusb_include_dir}/os/windows_winusb.h)
else()
    message(WARNING "Other Platform: ${CMAKE_SYSTEM_NAME}")
    return()
endif()
list(APPEND base_libusb_src_list ${libusb_platform_include_dir}/config.h)
get_cxx_files(${libusb_include_dir} src_list)
list(APPEND base_libusb_src_list ${src_list})

set(base_usb_src_list)
get_cxx_files(usb src_list)
list(APPEND base_usb_src_list ${src_list})

# 组装文件列表
set(base_usb_files
    ${base_libusb_src_list}
    ${base_usb_src_list}
    CACHE INTERNAL "")

# 打印文件列表
print_info(BODY "base usb files:")
foreach(filename ${base_usb_files})
    message("    " ${filename})
endforeach()
