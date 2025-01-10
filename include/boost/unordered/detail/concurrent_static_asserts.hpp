#ifndef BOOST_UNORDERED_DETAIL_CONCURRENT_STATIC_ASSERTS_HPP
#define BOOST_UNORDERED_DETAIL_CONCURRENT_STATIC_ASSERTS_HPP

#include <type_traits>
#include <iterator>

#define BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(F)                             \
  static_assert(boost::unordered::detail::is_invocable<F, value_type&>::value, \
    "The provided Callable must be invocable with value_type&");

#define BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(F)                       \
  static_assert(                                                               \
    boost::unordered::detail::is_invocable<F, value_type const&>::value,       \
    "The provided Callable must be invocable with value_type const&");

#if __cplusplus >= 202002L

#define BOOST_UNORDERED_STATIC_ASSERT_EXEC_POLICY(P)                           \
  static_assert(!std::is_base_of<std::execution::parallel_unsequenced_policy,  \
                  ExecPolicy>::value,                                          \
    "ExecPolicy must be sequenced.");                                          \
  static_assert(                                                               \
    !std::is_base_of<std::execution::unsequenced_policy, ExecPolicy>::value,   \
    "ExecPolicy must be sequenced.");

#else

#define BOOST_UNORDERED_STATIC_ASSERT_EXEC_POLICY(P)                           \
  static_assert(!std::is_base_of<std::execution::parallel_unsequenced_policy,  \
                  ExecPolicy>::value,                                          \
    "ExecPolicy must be sequenced.");

#endif

namespace boost {
namespace unordered {
namespace detail {

template<typename... Args>
struct last_type;

template<typename T>
struct last_type<T> {
    using type = T;
};

template<typename First, typename... Rest>
struct last_type<First, Rest...> {
    using type = typename last_type<Rest...>::type;
};

template<typename... Args>
struct penultimate_type;

template<typename T1, typename T2>
struct penultimate_type<T1, T2> {
    using type = T1;
};

template<typename First, typename... Rest>
struct penultimate_type<First, Rest...> {
    using type = typename penultimate_type<Rest...>::type;
};

#if __cplusplus < 201703L
template<typename...>
struct void_t_impl { using type = void; };

template<typename... Ts>
using void_t = typename void_t_impl<Ts...>::type;
#else
template<typename... Ts>
using void_t = std::void_t<Ts...>;
#endif

template<class, class... >
struct is_invocable_helper : std::false_type {};

template<class F, class... Args>
struct is_invocable_helper<
    void_t<decltype(std::declval<F>()(std::declval<Args>()...))>,
    F, Args...
> : std::true_type {};

template<class F, class... Args>
using is_invocable = is_invocable_helper<void, F, Args...>;

} // namespace detail
} // namespace unordered
} // namespace boost

#define BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_INVOCABLE(Arg, Args)           \
  BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(                                    \
    typename boost::unordered::detail::last_type<Arg, Args>::type)

#define BOOST_UNORDERED_STATIC_ASSERT_LAST_ARG_CONST_INVOCABLE(Arg, Args)     \
  BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(                              \
    typename boost::unordered::detail::last_type<Arg, Args>::type)

#define BOOST_UNORDERED_STATIC_ASSERT_PENULTIMATE_ARG_INVOCABLE(              \
  Arg1, Arg2, Args)                                                           \
  BOOST_UNORDERED_STATIC_ASSERT_INVOCABLE(                                    \
    typename boost::unordered::detail::penultimate_type<Arg1, Arg2,           \
      Args>::type)

#define BOOST_UNORDERED_STATIC_ASSERT_PENULTIMATE_ARG_CONST_INVOCABLE(        \
  Arg1, Arg2, Args)                                                           \
  BOOST_UNORDERED_STATIC_ASSERT_CONST_INVOCABLE(                              \
    typename boost::unordered::detail::penultimate_type<Arg1, Arg2,           \
      Args>::type)

#if __cplusplus >= 202002L && !defined(_MSC_VER)  // MSVC might need additional checks
#define BOOST_UNORDERED_STATIC_ASSERT_FWD_ITERATOR(Iterator)                   \
  static_assert(std::forward_iterator<Iterator>,                               \
    "The provided iterator must be at least forward");
#else
#define BOOST_UNORDERED_STATIC_ASSERT_FWD_ITERATOR(Iterator)                   \
  static_assert(                                                               \
    std::is_base_of<                                                           \
      std::forward_iterator_tag,                                               \
      typename std::iterator_traits<Iterator>::iterator_category>::value,      \
    "The provided iterator must be at least forward");
#endif

#define BOOST_UNORDERED_STATIC_ASSERT_KEY_COMPATIBLE_ITERATOR(Iterator)        \
  static_assert(                                                               \
    std::is_same<                                                              \
      typename std::iterator_traits<Iterator>::value_type,                     \
      key_type>::value ||                                                      \
    detail::are_transparent<                                                   \
      typename std::iterator_traits<Iterator>::value_type,                     \
      hasher, key_equal>::value,                                               \
    "The provided iterator must dereference to a compatible key value");

#define BOOST_UNORDERED_STATIC_ASSERT_BULK_VISIT_ITERATOR(Iterator)            \
  BOOST_UNORDERED_STATIC_ASSERT_FWD_ITERATOR(Iterator)                         \
  BOOST_UNORDERED_STATIC_ASSERT_KEY_COMPATIBLE_ITERATOR(Iterator)

#endif // BOOST_UNORDERED_DETAIL_CONCURRENT_STATIC_ASSERTS_HPP
