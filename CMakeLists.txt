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
        semantic_analysis/labelled_statements.cpp
        semantic_analysis/labelled_statements.h
        semantic_analysis/loop_labelling.cpp
        semantic_analysis/loop_labelling.h
        semantic_analysis/variable_resolution.cpp
        semantic_analysis/variable_resolution.h
        semantic_analysis.cpp
        semantic_analysis.h
        tacky.h
        utils.cpp
        utils.h
)
target_link_libraries(wccff PRIVATE
        ctre::ctre
        cxxopts::cxxopts
        fmt::fmt
)
target_include_directories(wccff PRIVATE .)

add_subdirectory(tests)
