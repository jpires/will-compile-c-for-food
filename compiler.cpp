#include "compiler.h"

#include "assembly_generation.h"
#include "code_emission.h"
#include "lexer.h"
#include "parser.h"
#include <filesystem>
#include <fmt/core.h>
#include <iostream>

namespace wccff {
bool compile(const std::filesystem::path &source_filename,
             const std::filesystem::path &output_filename,
             stop_phase stop)
{
    auto r = lexer::read_file(source_filename);

    if (r.has_value() == false)
    {
        fmt::print("Failed to read file {} with message ({})\n", source_filename.c_str(), r.error().message());
        return false;
    }

    auto le = lexer::lexer(r.value());
    if (le.has_value() == false)
    {
        auto error = le.error();
        fmt::print("Failed to lexer file {} with message({})\n", source_filename.c_str(), error.message);
        fmt::print("Error on line: {}:{}\n", error.location.line, error.location.column);
        fmt::print("With the input: {}\n", error.input);
        return false;
    }
    for (const auto i : le.value())
    {
        std::cout << "Type: " << i.t << "Value: " << i.c << std::endl;
    }

    if (stop == stop_phase::lexer)
    {
        return true;
    }

    //
    // Parser
    //

    parser::tokens tokens{ le.value() };
    auto parse_result = parse(tokens);
    if (parse_result.has_value() == false)
    {
        fmt::print("Failed to parse file {}\n", parse_result.error().message);
        return false;
    }

    if (stop == stop_phase::parser)
    {
        return true;
    }

    //
    // Codegen
    //

    auto codegen_result = assembly_generation::process(parse_result.value());

    if (stop == stop_phase::codegen)
    {
        return true;
    }

    //
    // Emit Assembly code
    //

    code_emission::process(output_filename, codegen_result);

    return true;
}
} // namespace wccff
