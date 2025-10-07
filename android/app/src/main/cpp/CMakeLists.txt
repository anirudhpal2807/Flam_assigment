cmake_minimum_required(VERSION 3.22.1)
project(edgeviewer_native)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(edgeviewer SHARED
    native-lib.cpp
    jni_bridge.cpp
    gl_bridge.cpp
    egl_wrapper.cpp
)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles-lib GLESv2)

# Locate repository root by walking up parent directories until both `jni` and `gl` exist.
# This is more robust than a fixed "../../../../../" path and avoids errors when CMake
# is invoked from different working directories.
set(REPO_SEARCH_DIR ${CMAKE_CURRENT_LIST_DIR})
set(_MAX_UP 10)
set(_UP_COUNT 0)
while(NOT (EXISTS "${REPO_SEARCH_DIR}/jni" AND EXISTS "${REPO_SEARCH_DIR}/gl") AND (_UP_COUNT LESS _MAX_UP))
    get_filename_component(REPO_SEARCH_DIR "${REPO_SEARCH_DIR}" PATH)
    math(EXPR _UP_COUNT "${_UP_COUNT} + 1")
endwhile()

if(EXISTS "${REPO_SEARCH_DIR}/jni" AND EXISTS "${REPO_SEARCH_DIR}/gl")
    set(REPO_ROOT "${REPO_SEARCH_DIR}")
else()
    message(FATAL_ERROR "Could not locate repo-level 'jni' and 'gl' directories from ${CMAKE_CURRENT_LIST_DIR}; inspected up to ${_UP_COUNT} levels.")
endif()

# Debug: print the resolved directories so Android/Gradle logs show which paths were used
message(STATUS "CMake: CMAKE_CURRENT_LIST_DIR=${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "CMake: REPO_ROOT=${REPO_ROOT}")

# Bring in the OpenCV pipeline and GL static libs from the repo
add_subdirectory(${REPO_ROOT}/jni ${CMAKE_CURRENT_BINARY_DIR}/jni_build)
add_subdirectory(${REPO_ROOT}/gl ${CMAKE_CURRENT_BINARY_DIR}/gl_build)

target_include_directories(edgeviewer PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${REPO_ROOT}/jni/src
    ${REPO_ROOT}/gl/src
)

target_link_libraries(edgeviewer
    ${log-lib}
    ${android-lib}
    ${egl-lib}
    ${gles-lib}
    edgeopencv
    edgegl
)

# Ensure PT_LOAD segments are aligned for 16 KB page-size devices (Android 15+ requirement)
# These options are supported by lld (NDK toolchain linker).
target_link_options(edgeviewer PRIVATE
    -Wl,-z,common-page-size=16384
    -Wl,-z,max-page-size=16384
)


