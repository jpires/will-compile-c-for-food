#include "../compiler.h"
#include "../lexer.h"

#include <catch2/catch_test_macros.hpp>
#include <string_view>

TEST_CASE("Lexer", "[lexer]")
{
    using wccff::lexer::file_location;
    using wccff::lexer::token_type;

    SECTION("No tokens")
    {
        SECTION("nullptr input")
        {
            // This string_view doesn't have any memory associated.
            // Any attempt to read data will crash
            std::string_view input;
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().empty());
        }
        SECTION("Empty input")
        {
            // This string_view has a string that only has a NULL char
            // Even though the length is zero, we can still read input[0] without
            // an out-of-bounds read.
            std::string_view input{ "" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().empty());
        }

        SECTION("Only contains spaces")
        {
            std::string_view input{ "      " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().empty());
        }
        SECTION("Only contains newlines")
        {
            std::string_view input{ "\n\n\n" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().empty());
        }
        SECTION("Only contains newlines and spaces")
        {
            std::string_view input{ "   \n\n  \n   " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().empty());
        }
    }

    SECTION("Location")
    {
        SECTION("Token at the begin of the input")
        {
            std::string_view input{ "a" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).loc == file_location{ 1, 1 });
        }
        SECTION("With spaces at before the token")
        {
            std::string_view input{ "   a" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).loc == file_location{ 1, 4 });
        }
        SECTION("With empty lines before the token")
        {
            std::string_view input{ "\n\n\na" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).loc == file_location{ 4, 1 });
        }
        SECTION("With empty lines and spaces before the token")
        {
            std::string_view input{ "\n\n\n    a" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).loc == file_location{ 4, 5 });
        }
        SECTION("Multiple tokens")
        {
            std::string_view input{ "\n\n\n    a\n\n  fo" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 2);
            REQUIRE(result.value().at(0).loc == file_location{ 4, 5 });
            REQUIRE(result.value().at(1).loc == file_location{ 6, 3 });
        }
    }

    SECTION("Multiple tokens")
    {
        SECTION("Separated with spaces")
        {
            std::string_view input{ "a 11 " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 2);
            REQUIRE(result.value().at(0).type == token_type::identifier);
            REQUIRE(result.value().at(0).text == "a");
            REQUIRE(result.value().at(1).type == token_type::int_constant);
            REQUIRE(result.value().at(1).text == "11");
        }

        SECTION("Separated by newlines")
        {
            std::string_view input{ "a\n\n11 " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 2);
            REQUIRE(result.value().at(0).type == token_type::identifier);
            REQUIRE(result.value().at(0).text == "a");
            REQUIRE(result.value().at(1).type == token_type::int_constant);
            REQUIRE(result.value().at(1).text == "11");
        }
    }
}

TEST_CASE("Identifiers", "[lexer]")
{
    SECTION("Only Chars")
    {
        SECTION("One char long")
        {
            std::string_view input{ "a" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::identifier);
            REQUIRE(result.value().at(0).text == "a");
        }

        SECTION("Five chars long")
        {
            std::string_view input{ "abcde" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::identifier);
            REQUIRE(result.value().at(0).text == "abcde");
        }
    }

    SECTION("Mix Chars and Digits")
    {
        SECTION("One char and One digit")
        {
            std::string_view input{ "a1" };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::identifier);
            REQUIRE(result.value().at(0).text == "a1");
        }

        SECTION("Five chars long and two digits")
        {
            std::string_view input{ "abcde45 " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::identifier);
            REQUIRE(result.value().at(0).text == "abcde45");
        }
    }
}

TEST_CASE("Keywords", "[lexer]")
{
    SECTION("break")
    {
        std::string_view input{ "break" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::break_keyword);
        REQUIRE(result.value().at(0).text == "break");
    }
    SECTION("continue")
    {
        std::string_view input{ "continue" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::continue_keyword);
        REQUIRE(result.value().at(0).text == "continue");
    }
    SECTION("do")
    {
        std::string_view input{ "do" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::do_keyword);
        REQUIRE(result.value().at(0).text == "do");
    }
    SECTION("double")
    {
        std::string_view input{ "double" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::double_keyword);
        REQUIRE(result.value().at(0).text == "double");
    }

    SECTION("else")
    {
        std::string_view input{ "else" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::else_keyword);
        REQUIRE(result.value().at(0).text == "else");
    }
    SECTION("extern")
    {
        std::string_view input{ "extern" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::extern_keyword);
        REQUIRE(result.value().at(0).text == "extern");
    }
    SECTION("for")
    {
        std::string_view input{ "for" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::for_keyword);
        REQUIRE(result.value().at(0).text == "for");
    }
    SECTION("goto")
    {
        std::string_view input{ "goto" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::goto_keyword);
        REQUIRE(result.value().at(0).text == "goto");
    }
    SECTION("if")
    {
        std::string_view input{ "if" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::if_keyword);
        REQUIRE(result.value().at(0).text == "if");
    }
    SECTION("long")
    {
        std::string_view input{ "long" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::long_keyword);
        REQUIRE(result.value().at(0).text == "long");
    }
    SECTION("void")
    {
        std::string_view input{ "void" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::void_keyword);
        REQUIRE(result.value().at(0).text == "void");
    }

    SECTION("int")
    {
        std::string_view input{ "int" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::int_keyword);
        REQUIRE(result.value().at(0).text == "int");
    }

    SECTION("return")
    {
        std::string_view input{ "return" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::return_keyword);
        REQUIRE(result.value().at(0).text == "return");
    }

    SECTION("signed")
    {
        std::string_view input{ "signed" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::signed_keyword);
        REQUIRE(result.value().at(0).text == "signed");
    }

    SECTION("static")
    {
        std::string_view input{ "static" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::static_keyword);
        REQUIRE(result.value().at(0).text == "static");
    }

    SECTION("unsigned")
    {
        std::string_view input{ "unsigned" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_keyword);
        REQUIRE(result.value().at(0).text == "unsigned");
    }

    SECTION("while")
    {
        std::string_view input{ "while" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::while_keyword);
        REQUIRE(result.value().at(0).text == "while");
    }
}

TEST_CASE("Constants", "[lexer]")
{

    SECTION("double")
    {
        auto build_token = [](std::string_view text) {
            return wccff::lexer::token{ wccff::lexer::token_type::floating_porint_constant, text, {} };
        };

        auto r1 = wccff::lexer::lexer("1. ");
        REQUIRE(r1.has_value());
        REQUIRE(r1.value().size() == 1);
        REQUIRE(r1.value().at(0) == build_token("1."));

        auto r2 = wccff::lexer::lexer("12. ");
        REQUIRE(r2.has_value());
        REQUIRE(r2.value().size() == 1);
        REQUIRE(r2.value().at(0) == build_token("12."));

        auto r3 = wccff::lexer::lexer("1.1 ");
        REQUIRE(r3.has_value());
        REQUIRE(r3.value().size() == 1);
        REQUIRE(r3.value().at(0) == build_token("1.1"));

        auto r4 = wccff::lexer::lexer("12.12 ");
        REQUIRE(r4.has_value());
        REQUIRE(r4.value().size() == 1);
        REQUIRE(r4.value().at(0) == build_token("12.12"));

        auto r5 = wccff::lexer::lexer("1.e1 ");
        REQUIRE(r5.has_value());
        REQUIRE(r5.value().size() == 1);
        REQUIRE(r5.value().at(0) == build_token("1.e1"));

        auto r6 = wccff::lexer::lexer("1.e-1 ");
        REQUIRE(r6.has_value());
        REQUIRE(r6.value().size() == 1);
        REQUIRE(r6.value().at(0) == build_token("1.e-1"));

        auto r7 = wccff::lexer::lexer("1.e+1 ");
        REQUIRE(r7.has_value());
        REQUIRE(r7.value().size() == 1);
        REQUIRE(r7.value().at(0) == build_token("1.e+1"));

        auto r8 = wccff::lexer::lexer("12.e12 ");
        REQUIRE(r8.has_value());
        REQUIRE(r8.value().size() == 1);
        REQUIRE(r8.value().at(0) == build_token("12.e12"));

        auto r9 = wccff::lexer::lexer("12.e-12 ");
        REQUIRE(r9.has_value());
        REQUIRE(r9.value().size() == 1);
        REQUIRE(r9.value().at(0) == build_token("12.e-12"));

        auto r10 = wccff::lexer::lexer("12.e+12 ");
        REQUIRE(r10.has_value());
        REQUIRE(r10.value().size() == 1);
        REQUIRE(r10.value().at(0) == build_token("12.e+12"));

        auto r11 = wccff::lexer::lexer("1.E1 ");
        REQUIRE(r11.has_value());
        REQUIRE(r11.value().size() == 1);
        REQUIRE(r11.value().at(0) == build_token("1.E1"));

        auto r12 = wccff::lexer::lexer("1.E-1 ");
        REQUIRE(r12.has_value());
        REQUIRE(r12.value().size() == 1);
        REQUIRE(r12.value().at(0) == build_token("1.E-1"));

        auto r13 = wccff::lexer::lexer("1.E+1 ");
        REQUIRE(r13.has_value());
        REQUIRE(r13.value().size() == 1);
        REQUIRE(r13.value().at(0) == build_token("1.E+1"));

        auto r14 = wccff::lexer::lexer("12.E12 ");
        REQUIRE(r14.has_value());
        REQUIRE(r14.value().size() == 1);
        REQUIRE(r14.value().at(0) == build_token("12.E12"));

        auto r15 = wccff::lexer::lexer("12.E-12 ");
        REQUIRE(r15.has_value());
        REQUIRE(r15.value().size() == 1);
        REQUIRE(r15.value().at(0) == build_token("12.E-12"));

        auto r16 = wccff::lexer::lexer("12.E+12 ");
        REQUIRE(r16.has_value());
        REQUIRE(r16.value().size() == 1);
        REQUIRE(r16.value().at(0) == build_token("12.E+12"));

        auto r17 = wccff::lexer::lexer(".1 ");
        REQUIRE(r17.has_value());
        REQUIRE(r17.value().size() == 1);
        REQUIRE(r17.value().at(0) == build_token(".1"));

        auto r18 = wccff::lexer::lexer(".12 ");
        REQUIRE(r18.has_value());
        REQUIRE(r18.value().size() == 1);
        REQUIRE(r18.value().at(0) == build_token(".12"));

        auto r19 = wccff::lexer::lexer(".1e1 ");
        REQUIRE(r19.has_value());
        REQUIRE(r19.value().size() == 1);
        REQUIRE(r19.value().at(0) == build_token(".1e1"));

        auto r20 = wccff::lexer::lexer(".1e-1 ");
        REQUIRE(r20.has_value());
        REQUIRE(r20.value().size() == 1);
        REQUIRE(r20.value().at(0) == build_token(".1e-1"));

        auto r21 = wccff::lexer::lexer(".1e+1 ");
        REQUIRE(r21.has_value());
        REQUIRE(r21.value().size() == 1);
        REQUIRE(r21.value().at(0) == build_token(".1e+1"));

        auto r22 = wccff::lexer::lexer(".1E1 ");
        REQUIRE(r22.has_value());
        REQUIRE(r22.value().size() == 1);
        REQUIRE(r22.value().at(0) == build_token(".1E1"));

        auto r23 = wccff::lexer::lexer(".1E-1 ");
        REQUIRE(r23.has_value());
        REQUIRE(r23.value().size() == 1);
        REQUIRE(r23.value().at(0) == build_token(".1E-1"));

        auto r24 = wccff::lexer::lexer(".1E+1 ");
        REQUIRE(r24.has_value());
        REQUIRE(r24.value().size() == 1);
        REQUIRE(r24.value().at(0) == build_token(".1E+1"));

        auto r25 = wccff::lexer::lexer(".12e12 ");
        REQUIRE(r25.has_value());
        REQUIRE(r25.value().size() == 1);
        REQUIRE(r25.value().at(0) == build_token(".12e12"));

        auto r26 = wccff::lexer::lexer(".12e-12 ");
        REQUIRE(r26.has_value());
        REQUIRE(r26.value().size() == 1);
        REQUIRE(r26.value().at(0) == build_token(".12e-12"));

        auto r27 = wccff::lexer::lexer(".12e+12 ");
        REQUIRE(r27.has_value());
        REQUIRE(r27.value().size() == 1);
        REQUIRE(r27.value().at(0) == build_token(".12e+12"));

        auto r28 = wccff::lexer::lexer(".12E12 ");
        REQUIRE(r28.has_value());
        REQUIRE(r28.value().size() == 1);
        REQUIRE(r28.value().at(0) == build_token(".12E12"));

        auto r29 = wccff::lexer::lexer(".12E-12 ");
        REQUIRE(r29.has_value());
        REQUIRE(r29.value().size() == 1);
        REQUIRE(r29.value().at(0) == build_token(".12E-12"));

        auto r30 = wccff::lexer::lexer(".12E+12 ");
        REQUIRE(r30.has_value());
        REQUIRE(r30.value().size() == 1);
        REQUIRE(r30.value().at(0) == build_token(".12E+12"));
    }
    SECTION("int")
    {
        SECTION("One digit")
        {
            std::string_view input{ "1 " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::int_constant);
            REQUIRE(result.value().at(0).text == "1");
        }
        SECTION("Four digits")
        {
            std::string_view input{ "1234 " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::int_constant);
            REQUIRE(result.value().at(0).text == "1234");
        }
    }

    SECTION("long")
    {
        SECTION("One digit, lowercase")
        {
            std::string_view input{ "1l " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::long_constant);
            REQUIRE(result.value().at(0).text == "1l");
        }

        SECTION("One digit, uppercase")
        {
            std::string_view input{ "1L " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::long_constant);
            REQUIRE(result.value().at(0).text == "1L");
        }

        SECTION("Four digits, lowercase")
        {
            std::string_view input{ "1234l " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::long_constant);
            REQUIRE(result.value().at(0).text == "1234l");
        }
        SECTION("Four digits, uppercase")
        {
            std::string_view input{ "1234L " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::long_constant);
            REQUIRE(result.value().at(0).text == "1234L");
        }
    }

    SECTION("unsigned int")
    {
        SECTION("One digit, lowercase")
        {
            std::string_view input{ "1u " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_int_constant);
            REQUIRE(result.value().at(0).text == "1u");
        }

        SECTION("One digit, uppercase")
        {
            std::string_view input{ "1U " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_int_constant);
            REQUIRE(result.value().at(0).text == "1U");
        }

        SECTION("Four digits, lowercase")
        {
            std::string_view input{ "1234u " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_int_constant);
            REQUIRE(result.value().at(0).text == "1234u");
        }
        SECTION("Four digits, uppercase")
        {
            std::string_view input{ "1234U " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_int_constant);
            REQUIRE(result.value().at(0).text == "1234U");
        }
    }

    SECTION("unsigned long")
    {
        SECTION("One digit, ul")
        {
            std::string_view input{ "1ul " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1ul");
        }

        SECTION("One digit, uL")
        {
            std::string_view input{ "1uL " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1uL");
        }

        SECTION("One digit, Ul")
        {
            std::string_view input{ "1Ul " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1Ul");
        }

        SECTION("One digit, UL")
        {
            std::string_view input{ "1UL " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1UL");
        }

        SECTION("One digit, lu")
        {
            std::string_view input{ "1lu " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1lu");
        }

        SECTION("One digit, Lu")
        {
            std::string_view input{ "1Lu " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1Lu");
        }

        SECTION("One digit, lU")
        {
            std::string_view input{ "1lU " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1lU");
        }

        SECTION("One digit, LU")
        {
            std::string_view input{ "1LU " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1LU");
        }

        SECTION("Four digits, ul")
        {
            std::string_view input{ "1234ul " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1234ul");
        }
        SECTION("Four digits, uL")
        {
            std::string_view input{ "1234uL " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1234uL");
        }
        SECTION("Four digits, Ul")
        {
            std::string_view input{ "1234Ul " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1234Ul");
        }
        SECTION("Four digits, UL")
        {
            std::string_view input{ "1234UL " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1234UL");
        }

        SECTION("Four digits, lu")
        {
            std::string_view input{ "1234lu " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1234lu");
        }
        SECTION("Four digits, Lu")
        {
            std::string_view input{ "1234Lu " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1234Lu");
        }

        SECTION("Four digits, lU")
        {
            std::string_view input{ "1234lU " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1234lU");
        }

        SECTION("Four digits, LU")
        {
            std::string_view input{ "1234LU " };
            auto result = wccff::lexer::lexer(input);
            REQUIRE(result.has_value());
            REQUIRE(result.value().size() == 1);
            REQUIRE(result.value().at(0).type == wccff::lexer::token_type::unsigned_long_constant);
            REQUIRE(result.value().at(0).text == "1234LU");
        }
    }
}

TEST_CASE("Operators", "[lexer]")
{
    SECTION("Negation Operator")
    {
        std::string_view input{ "-" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::negation_operator);
        REQUIRE(result.value().at(0).text == "-");
    }

    SECTION("Decrement Operator")
    {
        std::string_view input{ "--" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::decrement_operator);
        REQUIRE(result.value().at(0).text == "--");
    }

    SECTION("Bitwise Complement Operator")
    {
        std::string_view input{ "~" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::bitwise_complement_operator);
        REQUIRE(result.value().at(0).text == "~");
    }

    SECTION("Plus Operator")
    {
        std::string_view input{ "+" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::plus_operator);
        REQUIRE(result.value().at(0).text == "+");
    }

    SECTION("Multiplication Operator")
    {
        std::string_view input{ "*" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::multiplication_operator);
        REQUIRE(result.value().at(0).text == "*");
    }

    SECTION("Division Operator")
    {
        std::string_view input{ "/" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::division_operator);
        REQUIRE(result.value().at(0).text == "/");
    }

    SECTION("Remainder Operator")
    {
        std::string_view input{ "%" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::remainder_operator);
        REQUIRE(result.value().at(0).text == "%");
    }
    SECTION("Bitwise And Operator")
    {
        std::string_view input{ "&" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::bitwise_and_operator);
        REQUIRE(result.value().at(0).text == "&");
    }
    SECTION("Bitwise Or Operator")
    {
        std::string_view input{ "|" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::bitwise_or_operator);
        REQUIRE(result.value().at(0).text == "|");
    }
    SECTION("Bitwise Xor Operator")
    {
        std::string_view input{ "^" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::bitwise_xor_operator);
        REQUIRE(result.value().at(0).text == "^");
    }
    SECTION("Left Shift Operator")
    {
        std::string_view input{ "<<" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::left_shift_operator);
        REQUIRE(result.value().at(0).text == "<<");
    }
    SECTION("Right Shift Operator")
    {
        std::string_view input{ ">>" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::right_shift_operator);
        REQUIRE(result.value().at(0).text == ">>");
    }

    SECTION("Not Operator")
    {
        std::string_view input{ "!" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::not_operator);
        REQUIRE(result.value().at(0).text == "!");
    }
    SECTION("And Operator")
    {
        std::string_view input{ "&&" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::and_operator);
        REQUIRE(result.value().at(0).text == "&&");
    }
    SECTION("Or Operator")
    {
        std::string_view input{ "||" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::or_operator);
        REQUIRE(result.value().at(0).text == "||");
    }
    SECTION("Equals Operator")
    {
        std::string_view input{ "==" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::equals_operator);
        REQUIRE(result.value().at(0).text == "==");
    }
    SECTION("Not Equals Operator")
    {
        std::string_view input{ "!=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::not_equals_operator);
        REQUIRE(result.value().at(0).text == "!=");
    }
    SECTION("Less Than Operator")
    {
        std::string_view input{ "<" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::less_than_operator);
        REQUIRE(result.value().at(0).text == "<");
    }
    SECTION("Less Than or Equal Operator")
    {
        std::string_view input{ "<=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::less_than_or_equal_operator);
        REQUIRE(result.value().at(0).text == "<=");
    }
    SECTION("Greater Than Operator")
    {
        std::string_view input{ ">" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::greater_than_operator);
        REQUIRE(result.value().at(0).text == ">");
    }
    SECTION("Greater Than or Equal Operator")
    {
        std::string_view input{ ">=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::greater_than_or_equal_operator);
        REQUIRE(result.value().at(0).text == ">=");
    }
    SECTION("assignment Operator")
    {
        std::string_view input{ "=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::assignment_operator);
        REQUIRE(result.value().at(0).text == "=");
    }
    SECTION("Compound Plus Operator")
    {
        std::string_view input{ "+=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::compound_plus);
        REQUIRE(result.value().at(0).text == "+=");
    }
    SECTION("Compound Minus Operator")
    {
        std::string_view input{ "-=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::compound_minus);
        REQUIRE(result.value().at(0).text == "-=");
    }
    SECTION("Compound Multiplication Operator")
    {
        std::string_view input{ "*=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::compound_multiplication);
        REQUIRE(result.value().at(0).text == "*=");
    }
    SECTION("Compound Division Operator")
    {
        std::string_view input{ "/=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::compound_division);
        REQUIRE(result.value().at(0).text == "/=");
    }
    SECTION("Compound Remainder Operator")
    {
        std::string_view input{ "%=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::compound_remainder);
        REQUIRE(result.value().at(0).text == "%=");
    }
    SECTION("Compound Bitwise And Operator")
    {
        std::string_view input{ "&=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::compound_bitwise_and);
        REQUIRE(result.value().at(0).text == "&=");
    }
    SECTION("Compound Bitwise Or Operator")
    {
        std::string_view input{ "|=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::compound_bitwise_or);
        REQUIRE(result.value().at(0).text == "|=");
    }
    SECTION("Compound Bitwise Xor Operator")
    {
        std::string_view input{ "^=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::compound_bitwise_xor);
        REQUIRE(result.value().at(0).text == "^=");
    }
    SECTION("Compound Left Shift Operator")
    {
        std::string_view input{ "<<=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::compound_left_shift);
        REQUIRE(result.value().at(0).text == "<<=");
    }
    SECTION("Compound Right Shift Operator")
    {
        std::string_view input{ ">>=" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::compound_right_shift);
        REQUIRE(result.value().at(0).text == ">>=");
    }
    SECTION("Increment Operator")
    {
        std::string_view input{ "++" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::increment_operator);
        REQUIRE(result.value().at(0).text == "++");
    }
}

TEST_CASE("Other tokens", "[lexer]")
{
    SECTION("Semicolon")
    {
        std::string_view input{ ";" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::semicolon);
        REQUIRE(result.value().at(0).text == ";");
    }

    SECTION("Open Brace")
    {
        std::string_view input{ "{" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::open_brace);
        REQUIRE(result.value().at(0).text == "{");
    }

    SECTION("Close Brace")
    {
        std::string_view input{ "}" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::close_brace);
        REQUIRE(result.value().at(0).text == "}");
    }

    SECTION("Open Parenthesis")
    {
        std::string_view input{ "(" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::open_parenthesis);
        REQUIRE(result.value().at(0).text == "(");
    }

    SECTION("Close Parenthesis")
    {
        std::string_view input{ ")" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::close_parenthesis);
        REQUIRE(result.value().at(0).text == ")");
    }

    SECTION("Question Mark")
    {
        std::string_view input{ "?" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::question_mark);
        REQUIRE(result.value().at(0).text == "?");
    }

    SECTION("Colon")
    {
        std::string_view input{ ":" };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::colon);
        REQUIRE(result.value().at(0).text == ":");
    }

    SECTION("Comma")
    {
        std::string_view input{ "," };
        auto result = wccff::lexer::lexer(input);
        REQUIRE(result.has_value());
        REQUIRE(result.value().size() == 1);
        REQUIRE(result.value().at(0).type == wccff::lexer::token_type::comma);
        REQUIRE(result.value().at(0).text == ",");
    }
}
