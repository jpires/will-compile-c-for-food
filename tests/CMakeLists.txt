cmake_minimum_required(VERSION 3.29)

add_executable(unit_tests
        main.cpp

        assembly_generation_test.cpp
        lexer_test.cpp
        parser_test.cpp
        semantic_analysis_test.cpp
        tacky_test.cpp
        variable_map_test.cpp
        ../assembly_generation.cpp
        ../lexer.cpp
        ../parser.cpp
        ../semantic_analysis.cpp
        ../tacky.cpp
        ../semantic_analysis/labelled_statements.cpp
        ../semantic_analysis/loop_labelling.cpp
        ../semantic_analysis/variable_resolution.cpp
)
target_link_libraries(unit_tests PRIVATE
        Catch2::Catch2
        Catch2::Catch2WithMain
        ctre::ctre
        fmt::fmt
)
target_include_directories(unit_tests PRIVATE ..)

find_path(APPROVAL_TESTS_CPP_INCLUDE_DIRS "ApprovalTests.hpp")
target_include_directories(unit_tests PRIVATE ${APPROVAL_TESTS_CPP_INCLUDE_DIRS})
