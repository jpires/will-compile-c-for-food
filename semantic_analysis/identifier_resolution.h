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

#ifndef VARIABLE_RESOLUTION_H
#define VARIABLE_RESOLUTION_H

#include "parser.h"
#include "semantic_analysis.h"

namespace wccff::sema::variable_resolution {

auto process_assignment_node(const std::unique_ptr<parser::assignment_node> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::assignment_node>, semantic_error>;

auto process_binary_node(const std::unique_ptr<parser::binary_node> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::binary_node>, semantic_error>;

auto process_block(const parser::block &node, identifier_map &variable_map)
  -> std::expected<parser::block, semantic_error>;

auto process_block_item(const parser::block_item &node, identifier_map &variable_map)
  -> std::expected<parser::block_item, semantic_error>;

auto process_compound_statement(const std::unique_ptr<parser::compound_statement> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::compound_statement>, semantic_error>;

auto process_conditional_node(const std::unique_ptr<parser::conditional_node> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::conditional_node>, semantic_error>;

auto process_declaration(const parser::declaration &node, identifier_map &variable_map)
  -> std::expected<parser::declaration, semantic_error>;

auto process_do_while_statement(const std::unique_ptr<parser::do_while_statement> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::do_while_statement>, semantic_error>;

auto process_expression(const parser::expression &node, identifier_map &variable_map)
  -> std::expected<parser::expression, semantic_error>;

auto process_for_init(const parser::for_init &node, identifier_map &variable_map)
  -> std::expected<parser::for_init, semantic_error>;

auto process_for_statement(const std::unique_ptr<parser::for_statement> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::for_statement>, semantic_error>;

auto process_function_call(const std::unique_ptr<parser::function_call> &node, identifier_map &map)
  -> std::expected<std::unique_ptr<parser::function_call>, semantic_error>;

auto process_function_declaration(const parser::function_declaration &node, identifier_map &variable_map)
  -> std::expected<parser::function_declaration, semantic_error>;

auto process_if_node(const std::unique_ptr<parser::if_node> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::if_node>, semantic_error>;

auto process_init_declaration(const parser::init_declaration &node, identifier_map &variable_map)
  -> std::expected<parser::init_declaration, semantic_error>;

auto process_init_expression(const parser::init_expression &node, identifier_map &variable_map)
  -> std::expected<parser::init_expression, semantic_error>;

auto process_labelled_statement(const std::unique_ptr<parser::labelled_statement> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::labelled_statement>, semantic_error>;

auto process_program(const parser::program &node, identifier_map &variable_map)
  -> std::expected<parser::program, semantic_error>;

auto process_return_node(const parser::return_node &node, identifier_map &variable_map)
  -> std::expected<parser::return_node, semantic_error>;

auto process_statement(const parser::statement &node, identifier_map &variable_map)
  -> std::expected<parser::statement, semantic_error>;

auto process_unary_node(const std::unique_ptr<parser::unary_node> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::unary_node>, semantic_error>;

auto process_var(const parser::var &node, identifier_map &variable_map) -> std::expected<parser::var, semantic_error>;

auto process_variable_declaration(const parser::variable_declaration &node, identifier_map &variable_map)
  -> std::expected<parser::variable_declaration, semantic_error>;

auto process_while_statement(const std::unique_ptr<parser::while_statement> &node, identifier_map &variable_map)
  -> std::expected<std::unique_ptr<parser::while_statement>, semantic_error>;

} // namespace wccff::sema::variable_resolution

#endif // VARIABLE_RESOLUTION_H
