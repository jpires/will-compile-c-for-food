#ifndef CODE_EMISSION_H
#define CODE_EMISSION_H

#include "assembly_generation.h"
#include <filesystem>

namespace wccff::code_emission {

void process(const std::filesystem::path &output_file, const assembly_generation::program &p);

} // namespace wccff::code_emission

#endif // CODE_EMISSION_H
