#include "../parser.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("type", "[parser]")
{
    using wccff::int_type;
    using wccff::long_type;
    using wccff::void_type;

    using wccff::fun_type;

    REQUIRE(int_type{} == int_type{});
    REQUIRE(int_type{} != long_type{});
    REQUIRE(int_type{} != void_type{});

    REQUIRE(long_type{} != int_type{});
    REQUIRE(long_type{} == long_type{});
    REQUIRE(long_type{} != void_type{});

    REQUIRE(void_type{} != int_type{});
    REQUIRE(void_type{} != long_type{});
    REQUIRE(void_type{} == void_type{});

    fun_type t1{};
}
