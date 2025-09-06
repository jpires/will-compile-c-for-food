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
#include "symbol_table.h"
#include <unordered_map>
#include <vector>

namespace wccff::sema {

enum class scope_type
{
    file,
    inner,
};
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
        symbol(identifier name, identifier unique_name, linkage link)
          : linkage(link)
          , name(std::move(name))
          , unique_name(std::move(unique_name))
        {
        }

        linkage linkage;
        identifier name;
        identifier unique_name;
    };

    identifier_map() { m_map.emplace_back(); }
    identifier add(const identifier &name, linkage link = linkage::internal);
    void create_scope();
    void destroy_scope();
    std::optional<symbol> find(const identifier &name, scopes on_current_scope = scopes::all_scopes) const;

  private:
    identifier generate_unique_name(const identifier &name);

    std::vector<std::unordered_map<std::string, symbol>> m_map;
    int32_t m_counter = 0;
    int32_t m_scope_counter = 0;
};

std::expected<std::tuple<parser::program, symbol_table::symbol_table>, semantic_error> analyse(
  const parser::program &input);

std::expected<parser::expression, semantic_error> convert_by_assignment(const parser::expression &exp,
                                                                        const type &target);

bool is_lvalue(const parser::expression &e);

bool is_null_pointer_constant(const wccff::parser::expression &exp);

std::expected<type, semantic_error> get_common_pointer_type(const parser::expression &left,
                                                            const parser::expression &right);

} // namespace wccff::sema
/*
template<>
struct fmt::formatter<wccff::sema::identifier_map::linkage> : formatter<string_view>
{
    template<typename FormatContext>
    auto format(const wccff::sema::identifier_map::linkage link, FormatContext &ctx) const
    {
        using wccff::lexer::token_type;
        string_view str = "linkage::invalid";
        switch (link)
        {
            case wccff::sema::identifier_map::linkage::external:
                str = "linkage::external";
                break;
            case wccff::sema::identifier_map::linkage::internal:
                str = "linkage::internal";
                break;
            default:
                break;
        }

        return formatter<string_view>::format(str, ctx);
    }
};

template<>
struct fmt::formatter<wccff::sema::identifier_map::symbol> : formatter<string_view>
{
    template<typename FormatContext>
    auto format(const wccff::sema::identifier_map::symbol &s, FormatContext &ctx) const
    {
        auto str = fmt::format("{}:{}:{}", s.name, s.unique_name, s.linkage);
        return formatter<string_view>::format(str, ctx);
    }
};

template<>
struct fmt::formatter<wccff::sema::identifier_map> : formatter<string_view>
{
    template<typename FormatContext>
    auto format(const wccff::sema::identifier_map &map, FormatContext &ctx) const
    {
        for (const auto &s : map)
        {
        }
        auto str = fmt::format("{}:{}:{}", s.name, s.unique_name, s.linkage);
        return formatter<string_view>::format(str, ctx);
    }
};*/
#endif // SEMANTIC_ANALYSIS_H
