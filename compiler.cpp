#include "compiler.h"
#include "lexer.h"
#include <filesystem>
#include <fmt/core.h>
#include <iostream>

namespace wccff {
std::error_code compile(const std::filesystem::path &source_filename,
                        const std::filesystem::path &output_filename,
                        stop_phase stop)
{
    auto r = read_file(source_filename);

    if (r.has_value() == false)
    {
        std::cout << r.error().message() << std::endl;
        return r.error();
    }

    auto le = lexer(r.value());
    if (le.has_value() == false)
    {
        std::cout << le.error().message() << std::endl;
        return le.error();
    }
    for (const auto i : le.value())
    {
        std::cout << "Type: " << i.t << "Value: " << i.c << std::endl;
    }

    if (stop == stop_phase::lexer)
    {
        return {};
    }
    return {};
}
} // namespace wccff
