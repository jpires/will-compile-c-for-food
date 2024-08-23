#include <cxxopts.hpp>
#include <filesystem>
#include <fmt/core.h>
#include <iostream>

int run_preprocessor(const std::filesystem::path &source_file)
{
    auto cmd = fmt::format("gcc -E -P {}.c -o {}.i", source_file.stem().c_str(), source_file.stem().c_str());
    std::cout << cmd << std::endl;

    auto result = system(cmd.c_str());
    if (result != 0)
    {
        std::cerr << "Error while running the preprocessor" << std::endl;
    }

    return result;
}

int run_compiler(const std::filesystem::path &source_file)
{
    auto cmd = fmt::format("gcc -S {}.i -o {}.s", source_file.stem().c_str(), source_file.stem().c_str());
    std::cout << cmd << std::endl;

    auto result = system(cmd.c_str());
    if (result != 0)
    {
        std::cerr << "Error while running the compiler" << std::endl;
    }

    std::filesystem::remove_all(fmt::format("{}.i", source_file.stem().c_str()));

    return result;
}

int run_assembler_and_linker(const std::filesystem::path &source_file)
{
    auto cmd = fmt::format("gcc {}.s -o {}", source_file.stem().c_str(), source_file.stem().c_str());
    std::cout << cmd << std::endl;

    auto result = system(cmd.c_str());
    if (result != 0)
    {
        std::cerr << "Error while running the assembler" << std::endl;
    }

    std::filesystem::remove_all(fmt::format("{}.s", source_file.stem().c_str()));
    return result;
}

int main(int argc, char *argv[])
{
    cxxopts::Options options("iwccfl", "The compiler driver");

    // clang-format off
    options.add_options()
    ("lex", "Run the lexer", cxxopts::value<bool>()->default_value("false"))
    ("parse","Run the parser",cxxopts::value<bool>()->default_value("false"))
    ("codegen", "Run the codegen", cxxopts::value<bool>()->default_value("false"))
    ("S","Generate Assembly file",cxxopts::value<bool>()->default_value("false"))
    ("sourcefile", "The source file to process", cxxopts::value<std::string>())
    ("h,help", "Print usage");
    // clang-format on

    options.parse_positional({ "sourcefile" });

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("sourcefile") == 0)
    {
        std::cout << "No source file given" << std::endl;
        return 1;
    }

    auto source_filename = result["sourcefile"].as<std::string>();
    if (source_filename.ends_with(".c") == false)
    {
        std::cout << "The filename is wrong" << std::endl;
        return 1;
    }

    if (auto r = run_preprocessor(source_filename) != 0)
    {
        return r;
    }
    if (auto r = run_compiler(source_filename) != 0)
    {
        return r;
    }
    if (auto r = run_assembler_and_linker(source_filename) != 0)
    {
        return r;
    }

    return 0;
}
