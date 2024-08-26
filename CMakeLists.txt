cmake_minimum_required(VERSION 3.29)
project(will-compile-c-for-food)

set(CMAKE_CXX_STANDARD 23)

find_package(Catch2 CONFIG REQUIRED)
find_package(ctre CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(wccff
        assembly_generation.cpp
        assembly_generation.h
        code_emission.cpp
        code_emission.h
        compiler.cpp
        compiler.h
        driver.cpp
        lexer.cpp
        lexer.h
        parser.cpp
        parser.h
        tacky.cpp
        tacky.h
)
target_link_libraries(wccff PRIVATE
        ctre::ctre
        cxxopts::cxxopts
        fmt::fmt
)

add_subdirectory(tests)
