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

#ifndef LABELED_STATEMENTS_H
#define LABELED_STATEMENTS_H

#include "../parser.h"
#include "../semantic_analysis.h"

/**
 * Implements labelled statements and goto statement analysis.
 * It takes care of the following checks
 * * Validate that only one labelled statement per function has the same name
 * * Validates that the target of a goto, exists.
 *
 * It also changes the labels so that they are unique across a program.
 * By prefixing the label with the function name.
 */
namespace wccff::sema::labelled_statements {

class labelled_statement_map
{
  public:
    explicit labelled_statement_map(parser::identifier function_name)
      : m_function_name(std::move(function_name))
    {
    }
    std::expected<parser::identifier, semantic_error> add_label(const parser::identifier &l)
    {
        if (std::find(m_labels.begin(), m_labels.end(), l.name) != m_labels.end())
        {
            auto msg = fmt::format("label {} already exists in function {}", l.name, m_function_name.name);
            return std::unexpected{ semantic_error{ msg } };
        }
        m_labels.push_back(l.name);
        return generate_unique_name(l);
    }
    parser::identifier add_goto(const parser::identifier &l)
    {
        m_gotos.push_back(l.name);
        return generate_unique_name(l);
    }

    bool validate() const
    {
        for (const auto &g : m_gotos)
        {
            if (std::find(m_labels.begin(), m_labels.end(), g) == m_labels.end())
            {
                return false;
            }
        }
        return true;
    }

  private:
    parser::identifier generate_unique_name(const parser::identifier &l)
    {
        return parser::identifier(fmt::format("label_{}_{}", m_function_name.name, l.name));
    }
    parser::identifier m_function_name;
    std::vector<std::string> m_labels;
    std::vector<std::string> m_gotos;
};

auto process_block(const parser::block &node, labelled_statement_map &variable_map)
  -> std::expected<parser::block, semantic_error>;

auto process_block_item(const parser::block_item &node, labelled_statement_map &variable_map)
  -> std::expected<parser::block_item, semantic_error>;

auto process_compound_statement(const std::unique_ptr<parser::compound_statement> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::compound_statement>, semantic_error>;

auto process_declaration(const parser::declaration &node) -> std::expected<parser::declaration, semantic_error>;

auto process_do_while_statement(const std::unique_ptr<parser::do_while_statement> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::do_while_statement>, semantic_error>;

auto process_for_statement(const std::unique_ptr<parser::for_statement> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::for_statement>, semantic_error>;

auto process_goto_statement(const parser::goto_statement &node, labelled_statement_map &map)
  -> std::expected<parser::goto_statement, semantic_error>;

auto process_function(const parser::function &node) -> std::expected<parser::function, semantic_error>;

auto process_function_declaration(const parser::function_declaration &node)
  -> std::expected<parser::function_declaration, semantic_error>;

auto process_if_node(const std::unique_ptr<parser::if_node> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::if_node>, semantic_error>;

auto process_labelled_statement(const std::unique_ptr<parser::labelled_statement> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::labelled_statement>, semantic_error>;

auto process_program(const parser::program &node) -> std::expected<parser::program, semantic_error>;

auto process_statement(const parser::statement &node, labelled_statement_map &variable_map)
  -> std::expected<parser::statement, semantic_error>;

auto process_while_statement(const std::unique_ptr<parser::while_statement> &node, labelled_statement_map &map)
  -> std::expected<std::unique_ptr<parser::while_statement>, semantic_error>;

} // namespace wccff::sema::labelled_statements

#endif // LABELED_STATEMENTS_H
