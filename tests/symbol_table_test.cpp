#include "symbol_table.h"
#include <catch2/catch_test_macros.hpp>
#include <string_view>

TEST_CASE("Symbol Table", "[symbol_table]")
{
    using namespace wccff;

    wccff::symbol_table::symbol_table table;

    SECTION("local_attributes")
    {
        REQUIRE(table.get(parser::identifier{ "foo" }).has_value() == false);

        symbol_table::local_attributes attrs;
        table.add(parser::identifier{ "foo" }, symbol_table::int_type{}, attrs);

        auto s = table.get(parser::identifier{ "foo" });
        REQUIRE(s.has_value());
        REQUIRE(s.value().name == parser::identifier{ "foo" });
        REQUIRE(std::holds_alternative<symbol_table::int_type>(s.value().type));
        REQUIRE(std::holds_alternative<symbol_table::local_attributes>(s.value().attrs));
    }

    SECTION("static_attributes")
    {
        REQUIRE(table.get(parser::identifier{ "foo" }).has_value() == false);

        symbol_table::static_attributes attrs;
        attrs.is_global = true;
        attrs.init = symbol_table::no_initialiser{};
        table.add(parser::identifier{ "foo" }, symbol_table::int_type{}, attrs);

        auto s = table.get(parser::identifier{ "foo" });
        REQUIRE(s.has_value());
        REQUIRE(s.value().name == parser::identifier{ "foo" });
        REQUIRE(std::holds_alternative<symbol_table::int_type>(s.value().type));
        REQUIRE(std::holds_alternative<symbol_table::static_attributes>(s.value().attrs));
        auto new_attrs = std::get<symbol_table::static_attributes>(s.value().attrs);
        REQUIRE(new_attrs.is_global);
        REQUIRE(std::holds_alternative<symbol_table::no_initialiser>(new_attrs.init));

        // Replace symbol with an initial value
        attrs.init = symbol_table::initial{ 42 };
        table.add(parser::identifier{ "foo" }, symbol_table::int_type{}, attrs);
        auto s1 = table.get(parser::identifier{ "foo" });
        REQUIRE(s1.has_value());
        REQUIRE(s1.value().name == parser::identifier{ "foo" });
        REQUIRE(std::holds_alternative<symbol_table::int_type>(s1.value().type));
        REQUIRE(std::holds_alternative<symbol_table::static_attributes>(s1.value().attrs));
        auto new_attrs1 = std::get<symbol_table::static_attributes>(s1.value().attrs);
        REQUIRE(new_attrs1.is_global);
        REQUIRE(std::holds_alternative<symbol_table::initial>(new_attrs1.init));
        REQUIRE(std::get<symbol_table::initial>(new_attrs1.init).value == 42);
    }

    SECTION("func_attributes")
    {
        // Check that the identifier is not present in the table
        REQUIRE(table.get(parser::identifier{ "foo" }).has_value() == false);

        // Insert a new foo symbol without a body
        symbol_table::func_attributes attrs{};
        attrs.is_defined = false;
        attrs.is_global = true;

        table.add(parser::identifier{ "foo" }, symbol_table::int_type{}, attrs);

        auto s = table.get(parser::identifier{ "foo" });
        REQUIRE(s.has_value());
        REQUIRE(s.value().name == parser::identifier{ "foo" });
        REQUIRE(std::holds_alternative<symbol_table::int_type>(s.value().type));
        REQUIRE(std::holds_alternative<symbol_table::func_attributes>(s.value().attrs));
        auto new_attrs = std::get<symbol_table::func_attributes>(s.value().attrs);
        REQUIRE(new_attrs.is_global);
        REQUIRE(new_attrs.is_defined == false);

        // Replace the symbol foo with a body defined
        attrs.is_defined = true;
        table.add(parser::identifier{ "foo" }, symbol_table::int_type{}, attrs);

        auto s1 = table.get(parser::identifier{ "foo" });
        REQUIRE(s1.has_value());
        REQUIRE(s1.value().name == parser::identifier{ "foo" });
        REQUIRE(std::holds_alternative<symbol_table::int_type>(s1.value().type));
        REQUIRE(std::holds_alternative<symbol_table::func_attributes>(s1.value().attrs));
        auto new_attrs1 = std::get<symbol_table::func_attributes>(s1.value().attrs);
        REQUIRE(new_attrs1.is_global);
        REQUIRE(new_attrs1.is_defined);
    }
}
