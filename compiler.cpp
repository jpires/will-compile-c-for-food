#include "compiler.h"
#include "lexer.h"
#include <filesystem>
#include <fmt/core.h>
#include <iostream>

namespace wccff {
bool compile(const std::filesystem::path &source_filename,
             const std::filesystem::path &output_filename,
             stop_phase stop)
{
    auto r = read_file(source_filename);

    if (r.has_value() == false)
    {
        fmt::print("Failed to read file {} with message ({})\n", source_filename.c_str(), r.error().message());
        return false;
    }

    auto le = lexer(r.value());
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
    // Add next steps
    // Parse
    // Codegen
    return true;
}
} // namespace wccff
