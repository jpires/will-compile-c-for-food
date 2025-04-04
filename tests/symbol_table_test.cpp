#include "symbol_table.h"
#include <catch2/catch_test_macros.hpp>
#include <string_view>

TEST_CASE("Symbol Table", "[symbol_table]")
{
    using namespace wccff;

    wccff::symbol_table::symbol_table table;

    // Check that the identifier is not present in the table
    REQUIRE(table.get(parser::identifier{ "foo" }).has_value() == false);

    // Insert a new foo symbol without a body
    table.add(parser::identifier{ "foo" }, symbol_table::int_type{}, false);

    auto s = table.get(parser::identifier{ "foo" });
    REQUIRE(s.has_value());
    REQUIRE(s.value().name == parser::identifier{ "foo" });
    REQUIRE(s.value().has_body == false);
    REQUIRE(std::holds_alternative<symbol_table::int_type>(s.value().type));

    // Replace the symbol foo with a body defined
    table.add(parser::identifier{ "foo" }, symbol_table::int_type{}, true);

    auto s1 = table.get(parser::identifier{ "foo" });
    REQUIRE(s1.has_value());
    REQUIRE(s1.value().name == parser::identifier{ "foo" });
    REQUIRE(s1.value().has_body == true);
    REQUIRE(std::holds_alternative<symbol_table::int_type>(s1.value().type));
}
