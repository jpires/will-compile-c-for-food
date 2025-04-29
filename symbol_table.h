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

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include "parser.h"
#include <compare>
#include <string>
#include <unordered_map>

namespace wccff::symbol_table {

struct initial
{
    int value;
};
struct no_initialiser
{
};
struct tentative
{
};
using initial_value = std::variant<initial, no_initialiser, tentative>;

struct func_attributes
{
    bool is_defined;
    bool is_global;
};
struct local_attributes
{
};

struct static_attributes
{
    initial_value init;
    bool is_global;
};
using identifier_attributes = std::variant<func_attributes, local_attributes, static_attributes>;

struct func_type
{
    int param_num;
    func_type() = default;
    explicit func_type(size_t size)
      : param_num(size) {};

    bool operator==(const func_type &rhs) const = default;
};
struct int_type
{
    bool operator==(const int_type &rhs) const = default;
};
using type = std::variant<int_type, func_type>;
struct symbol
{
    identifier_attributes attrs;
    parser::identifier name;
    type type;
};

class symbol_table
{
  public:
    void add(const parser::identifier &name, type type, identifier_attributes attrs)
    {
        symbol s{ attrs, name, type };
        m_table[name.name] = s;
    }

    std::optional<symbol> get(const parser::identifier &name) const
    {
        auto it = m_table.find(name.name);
        if (it == m_table.end())
        {
            return std::nullopt;
        }

        return it->second;
    };

    std::unordered_map<std::string, symbol>::const_iterator begin() const { return m_table.begin(); }
    std::unordered_map<std::string, symbol>::const_iterator cbegin() const { return m_table.cbegin(); }
    std::unordered_map<std::string, symbol>::const_iterator end() const { return m_table.end(); }
    std::unordered_map<std::string, symbol>::const_iterator cend() const { return m_table.cend(); }

  private:
    std::unordered_map<std::string, symbol> m_table;
};
} // namespace wccff::symbol_table

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
struct fmt::formatter<wccff::symbol_table::symbol> : formatter<string_view>
{
    template<typename FormatContext>
    auto format(const wccff::symbol_table::symbol &s, FormatContext &ctx) const
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
#endif // SYMBOL_TABLE_H
