#ifndef VISITOR_H
#define VISITOR_H

namespace wccff {
template<class... Ts>
struct visitor : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
visitor(Ts...) -> visitor<Ts...>;
}
#endif //VISITOR_H
