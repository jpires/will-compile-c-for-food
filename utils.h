#ifndef UTILS_H
#define UTILS_H
#include <fmt/format.h>

namespace wccff {
template<typename... Args>
auto format_indented(int indent, fmt::format_string<Args...> format_str, Args &&...args)
{
    auto prefix = fmt::format("{:{}}", "", indent);
    auto msg = fmt::format(format_str, std::forward<Args>(args)...);
    return fmt::format("{}{}", prefix, msg);
}
} // namespace wccff

#endif // UTILS_H
