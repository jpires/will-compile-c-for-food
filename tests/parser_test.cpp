#include "../parser.h"

#include <__ranges/to.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Identifiers", "[parser]")
{
    SECTION("Parse Valid identifier")
    {
        wccff::file_location location{ 1, 2 };
        std::vector<wccff::token> tokens_vector;
        tokens_vector.emplace_back(wccff::token_type::identifier, "main", location);

        wccff::tokens tokens{ tokens_vector };

        auto r = wccff::parse_identifier(tokens);

        REQUIRE(r.has_value());
        REQUIRE(r.value().name == "main");
    }
}
