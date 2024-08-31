cmake_minimum_required(VERSION 3.29)

add_executable(unit_tests
        assembly_generation_test.cpp
        lexer_test.cpp
        parser_test.cpp
        tacky_test.cpp
        ../assembly_generation.cpp
        ../lexer.cpp
        ../parser.cpp
        ../tacky.cpp
)
target_link_libraries(unit_tests PRIVATE
        Catch2::Catch2
        Catch2::Catch2WithMain
        ctre::ctre
        fmt::fmt
)
