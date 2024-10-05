/*
 * Will Compile C for Food, a toy C compiler
 * Copyright (C) 2024  João Pires
 * https://github.com/jpires/will-compile-c-for-food
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SEMANTIC_ANALYSIS_H
#define SEMANTIC_ANALYSIS_H

#include "parser.h"

#include <unordered_map>
namespace wccff::sema {

struct semantic_error
{
    std::string message;
};
class variable_map
{
  public:
    void add(const parser::identifier &name, const parser::identifier &unique_name);
    bool contains(const parser::identifier &name) const;
    parser::identifier get_unique_name(const parser::identifier &name) const;

    parser::identifier generate_unique_name(const parser::identifier &name);

  private:
    std::unordered_map<std::string, std::string> m_map;
    int32_t m_counter = 0;
};

std::expected<parser::program, semantic_error> analyse(const parser::program &input);

std::expected<std::unique_ptr<parser::assignment_node>, semantic_error> resolve_assignment_node(
  const std::unique_ptr<parser::assignment_node> &node,
  variable_map &variable_map);

std::expected<parser::block_item, semantic_error> resolve_block_item(const parser::block_item &input,
                                                                     variable_map &variable_map);

std::expected<parser::declaration, semantic_error> resolve_declaration(const parser::declaration &input,
                                                                       variable_map &variable_map);

std::expected<parser::expression, semantic_error> resolve_expression(const parser::expression &input,
                                                                     variable_map &variable_map);

std::expected<parser::function, semantic_error> resolve_function(const parser::function &input,
                                                                 variable_map &variable_map);

std::expected<parser::return_node, semantic_error> resolve_return_node(const parser::return_node &input,
                                                                       variable_map &variable_map);

std::expected<parser::statement, semantic_error> resolve_statement(const parser::statement &input,
                                                                   variable_map &variable_map);
std::expected<std::unique_ptr<parser::binary_node>, semantic_error> resolve_binary_node(
  const std::unique_ptr<parser::binary_node> &node,
  variable_map &variable_map);
std::expected<std::unique_ptr<parser::unary_node>, semantic_error> resolve_unary_node(
  const std::unique_ptr<parser::unary_node> &node,
  variable_map &variable_map);
std::expected<parser::var, semantic_error> resolve_var(const parser::var &node, variable_map &variable_map);

std::expected<parser::program, semantic_error> variable_resolution(const parser::program &input,
                                                                   variable_map &variable_map);

} // namespace wccff::sema
#endif // SEMANTIC_ANALYSIS_H
