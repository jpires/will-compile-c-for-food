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
#include "common.h"
#include "parser.h"
#include <compare>
#include <string>
#include <unordered_map>

namespace wccff::symbol_table {

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

struct symbol
{
    symbol() = default;
    symbol(identifier_attributes attrs, parser::identifier name, type type)
      : attrs(attrs)
      , name(std::move(name))
      , type(std::move(type))
    {
    }
    symbol(const symbol &s)
    {
        attrs = s.attrs;
        name = s.name;
        type = copy_type(s.type);
    }
    symbol(symbol &&s) noexcept
    {
        attrs = s.attrs;
        name = s.name;
        type = std::move(s.type);
    }
    symbol &operator=(const symbol &s)
    {
        attrs = s.attrs;
        name = s.name;
        type = copy_type(s.type);
        return *this;
    }
    symbol &operator=(symbol &&s) noexcept
    {
        attrs = s.attrs;
        name = s.name;
        type = copy_type(s.type);
        return *this;
    }

    identifier_attributes attrs;
    parser::identifier name;
    type type;
};

class symbol_table
{
  public:
    void add(const parser::identifier &name, const type &type, identifier_attributes attrs)
    {
        symbol s{ attrs, name, copy_type(type) };
        m_table[name.name] = s;
    }

    std::optional<symbol> get(const parser::identifier &name) const
    {
        const auto it = m_table.find(name.name);
        if (it == m_table.end())
        {
            return std::nullopt;
        }

        return it->second;
    };

    std::size_t size() const { return m_table.size(); }

    std::unordered_map<std::string, symbol>::const_iterator begin() const { return m_table.begin(); }
    std::unordered_map<std::string, symbol>::const_iterator cbegin() const { return m_table.cbegin(); }
    std::unordered_map<std::string, symbol>::const_iterator end() const { return m_table.end(); }
    std::unordered_map<std::string, symbol>::const_iterator cend() const { return m_table.cend(); }

    parser::identifier current_processing_function;

  private:
    std::unordered_map<std::string, symbol> m_table;
};

struct obj_entry
{
    assembly_type asm_type;
    bool is_static;
    std::optional<int32_t> offset;
};

struct fun_entry
{
    bool is_defined;
};

using asm_symbtab_entry = std::variant<obj_entry, fun_entry>;

struct backend_symbol_table
{
    void add(const parser::identifier &name, const asm_symbtab_entry &entry) { m_table[name.name] = entry; }
    void build(const symbol_table &table);

    std::optional<asm_symbtab_entry> get(const parser::identifier &name) const
    {
        const auto it = m_table.find(name.name);
        if (it == m_table.end())
        {
            return std::nullopt;
        }

        return it->second;
    };

    int32_t get_symbol_offset(const parser::identifier &name)
    {
        const auto it = m_table.find(name.name);
        if (it == m_table.end())
        {
            throw std::runtime_error(fmt::format("Symbol {} not found in backend_symbol_table", name.name));
        }

        if (std::holds_alternative<fun_entry>(it->second))
        {
            throw std::runtime_error(fmt::format("Trying to get the offset of a function symbol"));
        }
        auto obj = std::get<obj_entry>(it->second);
        if (obj.is_static)
        {
            throw std::runtime_error(fmt::format("Trying to get the offset of a static symbol"));
        }

        if (obj.offset.has_value())
        {
            return obj.offset.value();
        }

        m_offset -= calculate_offset(obj.asm_type);
        obj.offset = m_offset;
        m_table[name.name] = obj;
        return m_offset;
    }
    int32_t get_current_offset() const { return m_offset; }

    std::size_t size() const { return m_table.size(); }

    void enter_function() { m_offset = 0; }

  private:
    assembly_type get_assembly_type(const wccff::type &t);
    int32_t calculate_offset(const assembly_type &assembly);

    std::unordered_map<std::string, asm_symbtab_entry> m_table;
    int32_t m_offset{ 0 };
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
