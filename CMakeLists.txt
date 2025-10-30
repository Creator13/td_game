cmake_minimum_required(VERSION 3.31)
project(td_game)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Enable testing
option(BUILD_TESTS "Build tests" OFF)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (MSVC)
    add_compile_options(/utf-8)
endif ()

add_subdirectory(src)
