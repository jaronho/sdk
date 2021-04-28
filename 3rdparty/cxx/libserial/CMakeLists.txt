cmake_minimum_required(VERSION 2.8.3)
project(libserial)

# Find catkin
find_package(catkin REQUIRED)

if(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
endif()

if(UNIX AND NOT APPLE)
    # If Linux, add rt and pthread
    set(rt_LIBRARIES rt)
    set(pthread_LIBRARIES pthread)
    catkin_package(
        LIBRARIES ${PROJECT_NAME}
        INCLUDE_DIRS libserial
        DEPENDS rt pthread
    )
else()
    # Otherwise normal call
    catkin_package(
        LIBRARIES ${PROJECT_NAME}
        INCLUDE_DIRS libserial
    )
endif()

## Sources
set(serial_SRCS
    libserial/serial.cpp
    libserial/serial.h
    libserial/v8stdint.h
)
if(APPLE)
    # If OSX
    list(APPEND serial_SRCS libserial/unix.cpp)
    list(APPEND serial_SRCS libserial/list_ports_osx.cpp)
elseif(UNIX)
    # If unix
    list(APPEND serial_SRCS libserial/unix.cpp)
    list(APPEND serial_SRCS libserial/list_ports_linux.cpp)
else()
    # If windows
    list(APPEND serial_SRCS libserial/win.cpp)
    list(APPEND serial_SRCS libserial/list_ports_win.cpp)
endif()

## Add libserial library
add_library(${PROJECT_NAME} ${serial_SRCS})
if(APPLE)
    target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} rt pthread)
else()
    target_link_libraries(${PROJECT_NAME} setupapi)
endif()

## Uncomment for example
add_executable(serial_example examples/serial_example.cpp)
add_dependencies(serial_example ${PROJECT_NAME})
target_link_libraries(serial_example ${PROJECT_NAME})

## Include headers
include_directories(libserial)

## Install executable
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)

## Install headers
install(FILES libserial/serial.h libserial/v8stdint.h
  DESTINATION ${CATKIN_GLOBAL_INCLUDE_DESTINATION}/libserial)

## Tests
if(CATKIN_ENABLE_TESTING)
    add_subdirectory(tests)
endif()
