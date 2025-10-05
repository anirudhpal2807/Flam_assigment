cmake_minimum_required(VERSION 3.22.1)
project(edgeviewer_gl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(edgegl STATIC
    src/gl_renderer.cpp
)

target_include_directories(edgegl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)


