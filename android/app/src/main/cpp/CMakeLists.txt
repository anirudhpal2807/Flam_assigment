cmake_minimum_required(VERSION 3.22.1)
project(edgeviewer_native)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(edgeviewer SHARED
    native-lib.cpp
    jni_bridge.cpp
)

find_library(log-lib log)

# Bring in the OpenCV pipeline static lib defined under /jni
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../jni ${CMAKE_CURRENT_BINARY_DIR}/jni_build)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../gl ${CMAKE_CURRENT_BINARY_DIR}/gl_build)

target_include_directories(edgeviewer PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../../jni/src
    ${CMAKE_CURRENT_LIST_DIR}/../../../gl/src
)

target_link_libraries(edgeviewer
    ${log-lib}
    edgeopencv
    edgegl
)


