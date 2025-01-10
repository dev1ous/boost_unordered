/* Copyright 2023-2024 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See https://www.boost.org/libs/unordered for library home page.
 */

#ifndef BOOST_UNORDERED_DETAIL_FOA_TUPLE_ROTATE_RIGHT_HPP
#define BOOST_UNORDERED_DETAIL_FOA_TUPLE_ROTATE_RIGHT_HPP

#include <tuple>
#include <utility>
#include <type_traits>

namespace boost{
namespace unordered{
namespace detail{
namespace foa{

template<std::size_t Offset, typename Tuple, std::size_t... Is>
constexpr auto tuple_rotate_right_impl(Tuple&& x, std::index_sequence<Is...>)
{
    constexpr std::size_t N = std::tuple_size<std::remove_reference_t<Tuple>>::value;
    return std::make_tuple(
        std::get<(Is + N - Offset) % N>(std::forward<Tuple>(x))...
    );
}

template<std::size_t Offset = 1, typename Tuple>
constexpr auto tuple_rotate_right(Tuple&& x)
{
    using RawTuple = std::remove_reference_t<Tuple>;
    constexpr std::size_t N = std::tuple_size<RawTuple>::value;
    return tuple_rotate_right_impl<Offset>(
        std::forward<Tuple>(x),
        std::make_index_sequence<N>{}
    );
}

} /* namespace foa */
} /* namespace detail */
} /* namespace unordered */
} /* namespace boost */

#endif
