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

#ifndef LOOP_LABELLING_H
#define LOOP_LABELLING_H

#include "parser.h"
#include "semantic_analysis.h"

namespace wccff::sema::loop_labelling {
auto process_block(const parser::block &node, const std::optional<parser::identifier> &label)
  -> std::expected<parser::block, semantic_error>;

auto process_block_item(const parser::block_item &node, const std::optional<parser::identifier> &label)
  -> std::expected<parser::block_item, semantic_error>;

auto process_break_statement(const std::optional<parser::identifier> &label)
  -> std::expected<parser::break_statement, semantic_error>;

auto process_compound_statement(const std::unique_ptr<parser::compound_statement> &node,
                                const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::compound_statement>, semantic_error>;

auto process_continue_statement(const std::optional<parser::identifier> &label)
  -> std::expected<parser::continue_statement, semantic_error>;

auto process_do_while_statement(const std::unique_ptr<parser::do_while_statement> &node,
                                const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::do_while_statement>, semantic_error>;

auto process_for_statement(const std::unique_ptr<parser::for_statement> &node,
                           const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::for_statement>, semantic_error>;

auto process_function(const parser::function &node) -> std::expected<parser::function, semantic_error>;

auto process_if_node(const std::unique_ptr<parser::if_node> &node, const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::if_node>, semantic_error>;

auto process_program(const parser::program &node) -> std::expected<parser::program, semantic_error>;

auto process_statement(const parser::statement &node, const std::optional<parser::identifier> &label)
  -> std::expected<parser::statement, semantic_error>;

auto process_while_statement(const std::unique_ptr<parser::while_statement> &node,
                             const std::optional<parser::identifier> &label)
  -> std::expected<std::unique_ptr<parser::while_statement>, semantic_error>;
} // namespace wccff::sema::loop_labelling

#endif // LOOP_LABELLING_H
