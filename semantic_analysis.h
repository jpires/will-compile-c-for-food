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
#include <vector>

namespace wccff::sema {

struct semantic_error
{
    std::string message;
};
class identifier_map
{
  public:
    enum class scopes
    {
        all_scopes,
        current_scope,
    };

    enum class linkage
    {
        external,
        internal,
    };

    struct symbol
    {
        symbol(parser::identifier name, parser::identifier unique_name, linkage link)
          : linkage(link)
          , name(std::move(name))
          , unique_name(std::move(unique_name))
        {
        }

        linkage linkage;
        parser::identifier name;
        parser::identifier unique_name;
    };

    identifier_map() { m_map.emplace_back(); }
    parser::identifier add(const parser::identifier &name, linkage link = linkage::internal);
    void create_scope();
    void destroy_scope();
    std::optional<symbol> find(const parser::identifier &name, scopes on_current_scope = scopes::all_scopes) const;

  private:
    parser::identifier generate_unique_name(const parser::identifier &name);

    std::vector<std::unordered_map<std::string, symbol>> m_map;
    int32_t m_counter = 0;
    int32_t m_scope_counter = 0;
};

std::expected<parser::program, semantic_error> analyse(const parser::program &input);

bool is_lvalue(const parser::expression &e);

} // namespace wccff::sema
#endif // SEMANTIC_ANALYSIS_H
