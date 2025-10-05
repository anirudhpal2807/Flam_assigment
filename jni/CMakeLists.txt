cmake_minimum_required(VERSION 3.22.1)
project(edgeviewer_jni)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(edgeopencv STATIC
    src/opencv_pipeline.cpp
)

target_include_directories(edgeopencv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Optional OpenCV integration (define OpenCV_DIR to enable)
set(EDGEVIEWER_USE_OPENCV OFF)
if (DEFINED OpenCV_DIR)
    find_package(OpenCV REQUIRED core imgproc)
    if (OpenCV_FOUND)
        target_include_directories(edgeopencv PUBLIC ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(edgeopencv ${OpenCV_LIBS})
        target_compile_definitions(edgeopencv PUBLIC EDGEVIEWER_USE_OPENCV=1)
        set(EDGEVIEWER_USE_OPENCV ON)
    endif()
endif()


