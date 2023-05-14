cmake_minimum_required(VERSION 3.24)

project(PEachy LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
    peachy
    src/main.cpp
    src/File.cpp
    src/PE.cpp
)

target_compile_features(
    peachy
    PRIVATE
    cxx_std_20
)

target_include_directories(
    peachy
    PRIVATE
    external
    src
)
