cmake_minimum_required(VERSION 3.22.1)
project(edgeviewer_native)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(edgeviewer SHARED
    native-lib.cpp
)

find_library(log-lib log)

target_link_libraries(edgeviewer ${log-lib})


