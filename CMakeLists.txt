cmake_minimum_required(VERSION 3.29)
project(will-compile-c-for-food)

set(CMAKE_CXX_STANDARD 23)

find_package(cxxopts CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(wccff driver.cpp)
target_link_libraries(wccff PRIVATE
        cxxopts::cxxopts
        fmt::fmt
)
