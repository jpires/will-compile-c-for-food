#include "common.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Types", "[common]")
{
    using wccff::copy_type;
    using wccff::double_type;
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

        REQUIRE(double_type{} == double_type{});
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

TEST_CASE("initial", "[common]")
{
    SECTION("double_initial")
    {
        using wccff::double_initial;

        auto pos_zero = double_initial(0.0);
        auto neg_zero = double_initial(-0.0);
        auto pos_one = double_initial(1.0);
        auto neg_one = double_initial(-1.0);

        REQUIRE(pos_one == pos_one);
        REQUIRE(pos_one != neg_one);

        REQUIRE(pos_one != pos_zero);

        REQUIRE(pos_zero == pos_zero);
        REQUIRE(neg_zero == neg_zero);
        REQUIRE(pos_zero != neg_zero);

        auto pos_zero_hash = std::hash<double_initial>{}(pos_zero);
        auto neg_zero_hash = std::hash<double_initial>{}(neg_zero);
        auto pos_one_hash = std::hash<double_initial>{}(pos_one);
        REQUIRE(pos_zero_hash != neg_zero_hash);
        REQUIRE(pos_zero_hash != pos_one_hash);
    }

    SECTION("int_initial")
    {
        using wccff::int_initial;

        auto zero = int_initial(0);
        auto one = int_initial(1);

        REQUIRE(zero == zero);
        REQUIRE(one == one);
        REQUIRE(zero != one);

        auto zero_hash = std::hash<int_initial>{}(zero);
        auto one_hash = std::hash<int_initial>{}(one);
        REQUIRE(zero_hash == zero_hash);
        REQUIRE(one_hash == one_hash);
        REQUIRE(zero_hash != one_hash);
    }

    SECTION("long_initial")
    {
        using wccff::long_initial;

        auto zero = long_initial(0);
        auto one = long_initial(1);

        REQUIRE(zero == zero);
        REQUIRE(one == one);
        REQUIRE(zero != one);

        auto zero_hash = std::hash<long_initial>{}(zero);
        auto one_hash = std::hash<long_initial>{}(one);
        REQUIRE(zero_hash == zero_hash);
        REQUIRE(one_hash == one_hash);
        REQUIRE(zero_hash != one_hash);
    }

    SECTION("unsigned_int_initial")
    {
        using wccff::unsigned_int_initial;

        auto zero = unsigned_int_initial(0);
        auto one = unsigned_int_initial(1);

        REQUIRE(zero == zero);
        REQUIRE(one == one);
        REQUIRE(zero != one);

        auto zero_hash = std::hash<unsigned_int_initial>{}(zero);
        auto one_hash = std::hash<unsigned_int_initial>{}(one);
        REQUIRE(zero_hash == zero_hash);
        REQUIRE(one_hash == one_hash);
        REQUIRE(zero_hash != one_hash);
    }

    SECTION("unsigned_long_initial")
    {
        using wccff::unsigned_long_initial;

        auto zero = unsigned_long_initial(0);
        auto one = unsigned_long_initial(1);

        REQUIRE(zero == zero);
        REQUIRE(one == one);
        REQUIRE(zero != one);

        auto zero_hash = std::hash<unsigned_long_initial>{}(zero);
        auto one_hash = std::hash<unsigned_long_initial>{}(one);
        REQUIRE(zero_hash == zero_hash);
        REQUIRE(one_hash == one_hash);
        REQUIRE(zero_hash != one_hash);
    }

    SECTION("initial")
    {
        using wccff::double_initial;
        using wccff::initial;
        using wccff::int_initial;
        using wccff::long_initial;
        using wccff::unsigned_int_initial;
        using wccff::unsigned_long_initial;

        auto double_init = initial{ double_initial(0.0) };
        auto int_init = initial{ int_initial(0) };
        auto long_init = initial{ long_initial(0) };
        auto unsigned_int_init = initial{ unsigned_int_initial(0) };
        auto unsigned_long_init = initial{ unsigned_long_initial(0) };

        REQUIRE(double_init == double_init);
        REQUIRE(int_init == int_init);
        REQUIRE(long_init == long_init);
        REQUIRE(unsigned_int_init == unsigned_int_init);
        REQUIRE(unsigned_long_init == unsigned_long_init);

        REQUIRE(double_init != int_init);
        REQUIRE(double_init != long_init);
        REQUIRE(double_init != unsigned_int_init);
        REQUIRE(double_init != unsigned_long_init);

        REQUIRE(int_init != double_init);
        REQUIRE(int_init != long_init);
        REQUIRE(int_init != unsigned_int_init);
        REQUIRE(int_init != unsigned_long_init);

        REQUIRE(long_init != double_init);
        REQUIRE(long_init != int_init);
        REQUIRE(long_init != unsigned_int_init);
        REQUIRE(long_init != unsigned_long_init);

        REQUIRE(unsigned_int_init != double_init);
        REQUIRE(unsigned_int_init != int_init);
        REQUIRE(unsigned_int_init != long_init);
        REQUIRE(unsigned_int_init != unsigned_long_init);

        REQUIRE(unsigned_long_init != double_init);
        REQUIRE(unsigned_long_init != int_init);
        REQUIRE(unsigned_long_init != long_init);
        REQUIRE(unsigned_long_init != unsigned_int_init);

        REQUIRE(std::hash<initial>{}(double_init) == std::hash<double_initial>{}(double_initial(0.0)));
        REQUIRE(std::hash<initial>{}(int_init) == std::hash<int_initial>{}(int_initial(0)));
        REQUIRE(std::hash<initial>{}(long_init) == std::hash<long_initial>{}(long_initial(0)));
        REQUIRE(std::hash<initial>{}(unsigned_int_init) == std::hash<unsigned_int_initial>{}(unsigned_int_initial(0)));
        REQUIRE(std::hash<initial>{}(unsigned_long_init) ==
                std::hash<unsigned_long_initial>{}(unsigned_long_initial(0)));
    }
}
