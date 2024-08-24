cmake_minimum_required(VERSION 3.29)

add_executable(unit_tests
        lexer_test.cpp
        parser_test.cpp
        ../lexer.cpp
        ../parser.cpp
)
target_link_libraries(unit_tests PRIVATE
        Catch2::Catch2
        Catch2::Catch2WithMain
        ctre::ctre
        fmt::fmt
)
