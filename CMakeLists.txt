cmake_minimum_required(VERSION 3.29)
project(will-compile-c-for-food)

set(CMAKE_CXX_STANDARD 23)

find_package(Catch2 CONFIG REQUIRED)
find_package(ctre CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(wccff
        compiler.cpp
        driver.cpp
        lexer.cpp
)
target_link_libraries(wccff PRIVATE
        ctre::ctre
        cxxopts::cxxopts
        fmt::fmt
)

add_subdirectory(tests)
