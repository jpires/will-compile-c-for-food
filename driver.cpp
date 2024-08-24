#include "compiler.h"
#include <cxxopts.hpp>
#include <filesystem>
#include <fmt/core.h>
#include <iostream>

std::filesystem::path get_preprocessor_path(const std::filesystem::path &source_file)
{
    return source_file.parent_path() / fmt::format("{}.i", source_file.filename().stem().c_str());
}
std::filesystem::path get_assembly_path(const std::filesystem::path &source_file)
{
    return source_file.parent_path() / fmt::format("{}.s", source_file.filename().stem().c_str());
}
std::filesystem::path get_binary_path(const std::filesystem::path &source_file)
{
    return source_file.parent_path() / fmt::format("{}", source_file.filename().stem().c_str());
}
int run_preprocessor(const std::filesystem::path &source_file)
{
    auto dst_file = get_preprocessor_path(source_file);

    auto cmd = fmt::format("gcc -E -P {} -o {}", source_file.c_str(), dst_file.c_str());
    std::cout << cmd << std::endl;

    auto result = system(cmd.c_str());
    if (result != 0)
    {
        std::cerr << "Error while running the preprocessor" << std::endl;
    }

    return result;
}

int run_compiler(const std::filesystem::path &source_file, wccff::stop_phase stop_phase)
{
    auto src_file = get_preprocessor_path(source_file);
    auto dst_file = get_assembly_path(source_file);

    auto ec = wccff::compile(src_file, dst_file, stop_phase);
    if (ec)
    {
        std::filesystem::remove_all(fmt::format("{}.i", source_file.stem().c_str()));
        std::cerr << "Error while running the compiler: " << ec.message() << std::endl;
        return 1;
    }
    std::filesystem::remove_all(src_file);

    return 0;
}

int run_assembler_and_linker(const std::filesystem::path &source_file, wccff::stop_phase stop_phase)
{
    auto src_file = get_assembly_path(source_file);

    if (stop_phase != wccff::stop_phase::no_stop)
    {
        std::filesystem::remove_all(src_file);
        return 0;
    }

    auto dst_file = get_binary_path(source_file);
    auto cmd = fmt::format("gcc {} -o {}", src_file.c_str(), dst_file.c_str());
    std::cout << cmd << std::endl;

    auto result = system(cmd.c_str());
    if (result != 0)
    {
        std::cerr << "Error while running the assembler" << std::endl;
    }

    std::filesystem::remove_all(src_file);
    return result;
}

int main(int argc, char *argv[])
{
    cxxopts::Options options("iwccfl", "The compiler driver");

    // clang-format off
    options.add_options()
    ("lex", "Run the lexer", cxxopts::value<bool>()->implicit_value("true"))
    ("parse","Run the parser",cxxopts::value<bool>()->implicit_value("true"))
    ("codegen", "Run the codegen", cxxopts::value<bool>()->implicit_value("true"))
    ("S","Generate Assembly file",cxxopts::value<bool>()->implicit_value("true"))
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

    wccff::stop_phase stop_phase = wccff::stop_phase::no_stop;
    if (result["lex"].as<bool>())
    {
        stop_phase = wccff::stop_phase::lexer;
    }
    if (result["parse"].as<bool>())
    {
        stop_phase = wccff::stop_phase::parser;
    }
    if (result["codegen"].as<bool>())
    {
        stop_phase = wccff::stop_phase::codegen;
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
    if (auto r = run_compiler(source_filename, stop_phase) != 0)
    {
        return r;
    }
    if (auto r = run_assembler_and_linker(source_filename, stop_phase) != 0)
    {
        return r;
    }

    return 0;
}
