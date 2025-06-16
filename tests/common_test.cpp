#include "common.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Types", "[common]")
{
    using wccff::copy_type;
    using wccff::fun_type;
    using wccff::int_type;
    using wccff::long_type;
    using wccff::type;
    using wccff::unsigned_int_type;
    using wccff::unsigned_long_type;

    SECTION("operator==")
    {
        std::vector<type> p1;
        p1.emplace_back(int_type{});
        std::vector<type> p2;
        p2.emplace_back(long_type{});
        std::vector<type> p3;
        p3.emplace_back(int_type{});
        p3.emplace_back(long_type{});

        REQUIRE(int_type{} == int_type{});
        REQUIRE(long_type{} == long_type{});
        REQUIRE(unsigned_int_type{} == unsigned_int_type{});
        REQUIRE(unsigned_long_type{} == unsigned_long_type{});

        fun_type fun1{ .params = {}, .return_type = int_type{} };
        fun_type fun2{ .params = {}, .return_type = int_type{} };
        fun_type fun3{ .params = {}, .return_type = long_type{} };
        fun_type fun4{ .params = std::move(p1), .return_type = int_type{} };
        fun_type fun5{ .params = std::move(p2), .return_type = int_type{} };
        fun_type fun6{ .params = std::move(p3), .return_type = int_type{} };

        REQUIRE(fun1 == fun1);
        REQUIRE(fun1 == fun2);
        REQUIRE(fun2 == fun2);
        REQUIRE(fun3 == fun3);
        REQUIRE(fun4 == fun4);
        REQUIRE(fun5 == fun5);
        REQUIRE(fun6 == fun6);

        REQUIRE(fun1 != fun3);
        REQUIRE(fun1 != fun4);
        REQUIRE(fun4 != fun5);
        REQUIRE(fun4 != fun6);
    }

    SECTION("get_common_type")
    {
        using wccff::get_common_type;

        REQUIRE(get_common_type(int_type{}, int_type{}) == int_type{});
        REQUIRE(get_common_type(long_type{}, long_type{}) == long_type{});
        REQUIRE(get_common_type(unsigned_int_type{}, unsigned_int_type{}) == unsigned_int_type{});
        REQUIRE(get_common_type(unsigned_long_type{}, unsigned_long_type{}) == unsigned_long_type{});

        REQUIRE(get_common_type(int_type{}, long_type{}) == long_type{});
        REQUIRE(get_common_type(long_type{}, int_type{}) == long_type{});

        REQUIRE(get_common_type(int_type{}, unsigned_int_type{}) == unsigned_int_type{});
        REQUIRE(get_common_type(unsigned_int_type{}, int_type{}) == unsigned_int_type{});

        REQUIRE(get_common_type(int_type{}, unsigned_long_type{}) == unsigned_long_type{});
        REQUIRE(get_common_type(unsigned_long_type{}, int_type{}) == unsigned_long_type{});

        REQUIRE(get_common_type(long_type{}, unsigned_int_type{}) == long_type{});
        REQUIRE(get_common_type(unsigned_int_type{}, long_type{}) == long_type{});

        REQUIRE(get_common_type(long_type{}, unsigned_long_type{}) == unsigned_long_type{});
        REQUIRE(get_common_type(unsigned_long_type{}, long_type{}) == unsigned_long_type{});

        REQUIRE(get_common_type(unsigned_int_type{}, unsigned_long_type{}) == unsigned_long_type{});
        REQUIRE(get_common_type(unsigned_long_type{}, unsigned_int_type{}) == unsigned_long_type{});
    }
}

// int, long, uint, ulong

// int, int     ->  Done
// int, long    ->  Done
// int, uint    ->  Done
// int, ulong   ->  Done

// long, long   ->  Done
// long, uint
// long, ulong

// uint, uint   ->  Done
// uint, ulong

// ulong, ulong ->  Done
