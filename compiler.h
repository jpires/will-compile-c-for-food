#ifndef COMPILER_H
#define COMPILER_H

#include <filesystem>
#include <string_view>
#include <system_error>

namespace wccff {
enum class stop_phase
{
  no_stop,
  lexer,
  parser,
  codegen
};

std::error_code compile(const std::filesystem::path &source_filename,
                        const std::filesystem::path &output_filename,
                        stop_phase stop);
}
#endif // COMPILER_H
