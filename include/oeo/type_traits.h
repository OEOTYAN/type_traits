#pragma once

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace oeo {

template <class...>
constexpr bool always_false = false;


template <class T, template <class...> class Z, class... Ts>
concept require = Z<T, Ts...>::value;


template <size_t N, class T, class... Ts>
struct get_type {
    using type = typename get_type<N - 1, Ts...>::type;
};

template <class T, class... Ts>
struct get_type<0, T, Ts...> {
    using type = T;
};

template <size_t N, class... Ts>
using get_type_t = typename get_type<N, Ts...>::type;


template <class T, class U, class... Ts>
struct max_type {
    using type = typename max_type<T, typename max_type<U, Ts...>::type>::type;
};

template <class T, class U>
struct max_type<T, U> {
    using type = typename std::conditional<(sizeof(T) < sizeof(U)), U, T>::type;
};


template <class T, class... Ts>
struct index_of;

template <class T, class... Ts>
struct index_of<T, T, Ts...> : std::integral_constant<size_t, 0> {};

template <class T, class U, class... Ts>
struct index_of<T, U, Ts...> : std::integral_constant<size_t, 1 + index_of<T, Ts...>::value> {};


template <class... Ts>
constexpr bool is_all_same_v = true;

template <class T, class... Ts>
constexpr bool is_all_same_v<T, Ts...> = (std::is_same_v<T, Ts> && ...);

template <class... Ts>
struct is_all_same : std::bool_constant<is_all_same_v<Ts...>> {};


template <class>
struct function_traits;

template <class Ret, class... Args>
struct function_traits<Ret (*)(Args...)> : function_traits<Ret(Args...)> {};

template <class F>
struct function_traits : function_traits<decltype(&F::operator())> {};

#ifndef O_BUILD_FUNCTION_SIGNATURE
#define O_BUILD_FUNCTION_SIGNATURE(CVREF, NOEXCEPT)                                                                    \
    template <class Ret, class... Args>                                                                                \
    struct function_traits<Ret(Args...) CVREF noexcept(NOEXCEPT)> {                                                    \
        using function_type = Ret(Args...);                                                                            \
        using return_type   = Ret;                                                                                     \
        template <class T>                                                                                             \
        using cvref                               = T       CVREF;                                                     \
        static constexpr bool is_noexcept         = NOEXCEPT;                                                          \
        static constexpr bool is_const            = std::is_const_v<cvref<int>>;                                       \
        static constexpr bool is_volatile         = std::is_volatile_v<cvref<int>>;                                    \
        static constexpr bool is_reference        = std::is_reference_v<cvref<int>>;                                   \
        static constexpr bool is_lvalue_reference = std::is_lvalue_reference_v<cvref<int>>;                            \
        static constexpr bool is_rvalue_reference = std::is_rvalue_reference_v<cvref<int>>;                            \
        template <template <class...> class T>                                                                         \
        using parameters = T<Args...>;                                                                                 \
    };                                                                                                                 \
    template <class Ret, class Cls, class... Args>                                                                     \
    struct function_traits<Ret (Cls::*)(Args...) CVREF noexcept(NOEXCEPT)>                                             \
    : function_traits<Ret(Args...) CVREF noexcept(NOEXCEPT)> {                                                         \
        using this_type = Cls;                                                                                         \
    };

O_BUILD_FUNCTION_SIGNATURE(, false)
O_BUILD_FUNCTION_SIGNATURE(const, false)
O_BUILD_FUNCTION_SIGNATURE(volatile, false)
O_BUILD_FUNCTION_SIGNATURE(const volatile, false)
O_BUILD_FUNCTION_SIGNATURE(, true)
O_BUILD_FUNCTION_SIGNATURE(const, true)
O_BUILD_FUNCTION_SIGNATURE(volatile, true)
O_BUILD_FUNCTION_SIGNATURE(const volatile, true)
O_BUILD_FUNCTION_SIGNATURE(&, false)
O_BUILD_FUNCTION_SIGNATURE(const&, false)
O_BUILD_FUNCTION_SIGNATURE(volatile&, false)
O_BUILD_FUNCTION_SIGNATURE(const volatile&, false)
O_BUILD_FUNCTION_SIGNATURE(&, true)
O_BUILD_FUNCTION_SIGNATURE(const&, true)
O_BUILD_FUNCTION_SIGNATURE(volatile&, true)
O_BUILD_FUNCTION_SIGNATURE(const volatile&, true)
O_BUILD_FUNCTION_SIGNATURE(&&, false)
O_BUILD_FUNCTION_SIGNATURE(const&&, false)
O_BUILD_FUNCTION_SIGNATURE(volatile&&, false)
O_BUILD_FUNCTION_SIGNATURE(const volatile&&, false)
O_BUILD_FUNCTION_SIGNATURE(&&, true)
O_BUILD_FUNCTION_SIGNATURE(const&&, true)
O_BUILD_FUNCTION_SIGNATURE(volatile&&, true)
O_BUILD_FUNCTION_SIGNATURE(const volatile&&, true)
#undef O_BUILD_FUNCTION_SIGNATURE
#endif


template <class T, class... Ts>
constexpr bool is_one_of_v = (std::is_same_v<T, Ts> || ...);

template <class T, class... Ts>
struct is_one_of : std::bool_constant<is_one_of_v<T, Ts...>> {};

template <class T, class... Ts>
concept one_of = is_one_of_v<T, Ts...>;


template <class T, class U>
constexpr bool is_within_variadic_v = false;

template <class T, template <class...> class U, class... Ts>
constexpr bool is_within_variadic_v<T, U<Ts...>> = is_one_of_v<T, Ts...>;

template <class T, class U>
struct is_within_variadic : std::bool_constant<is_within_variadic_v<T, U>> {};

template <class T, class U>
concept within_variadic = is_within_variadic_v<T, U>;


template <class T, template <class...> class Z>
constexpr bool is_specialization_of_v = false;

template <template <class...> class Z, class... Ts>
constexpr bool is_specialization_of_v<Z<Ts...>, Z> = true;

template <class T, template <class...> class Z>
struct is_specialization_of : std::bool_constant<is_specialization_of_v<T, Z>> {};

template <class T, template <class...> class Z>
concept specializes = is_specialization_of_v<T, Z>;


template <template <class...> class T, class... Ts>
void derived_from_specialization_impl(T<Ts...> const&);

template <class T, template <class...> class Z>
constexpr bool is_derived_from_specialization_of_v =
    requires(std::remove_cvref_t<T> const& t) { ::oeo::derived_from_specialization_impl<Z>(t); };

template <class T, template <class...> class Z>
struct is_derived_from_specialization_of : std::bool_constant<is_derived_from_specialization_of_v<T, Z>> {};

template <class T, template <class...> class Z>
concept derived_from_specializes = is_derived_from_specialization_of_v<T, Z>;


template <class T>
constexpr bool is_virtual_cloneable_v =
    std::is_polymorphic_v<T> && (requires(std::remove_cv_t<T> const& t) {
        static_cast<T*>(t.clone());
    } || requires(std::remove_cv_t<T> const& t) { static_cast<T*>(t.clone().release()); });

template <class T>
struct is_virtual_cloneable : std::bool_constant<is_virtual_cloneable_v<T>> {};

template <class T>
concept virtual_cloneable = is_virtual_cloneable_v<T>;


template <class T>
constexpr bool is_awaiter_v = requires(std::remove_cvref_t<T>& t) {
    { t.await_ready() } -> std::same_as<bool>;
    t.await_resume();
};
template <class T>
struct is_awaiter : std::bool_constant<is_awaiter_v<T>> {};

template <class T>
concept awaiter = is_awaiter_v<T>;


template <class T>
constexpr bool is_awaitable_v = requires(T t) {
    { t.operator co_await() } -> awaiter;
} || requires(T t) {
    { operator co_await(t) } -> awaiter;
} || is_awaiter_v<T>;

template <class T>
struct is_awaitable : std::bool_constant<is_awaitable_v<T>> {};

template <class T>
concept awaitable = is_awaitable_v<T>;


template <class T>
constexpr bool is_range_loopable_v = std::is_bounded_array_v<std::remove_cvref_t<T>> || requires(T t) {
    t.begin();
    t.end();
} || requires(T t) {
    begin(t);
    end(t);
};
template <class T>
struct is_range_loopable : std::bool_constant<is_range_loopable_v<T>> {};

template <class T>
concept range_loopable = is_range_loopable_v<T>;


template <class T>
constexpr bool is_associative_v = is_range_loopable_v<T> && requires {
    typename std::remove_cvref_t<T>::key_type;
    typename std::remove_cvref_t<T>::mapped_type;
};
template <class T>
struct is_associative : std::bool_constant<is_associative_v<T>> {};

template <class T>
concept associative = is_associative_v<T>;


template <class T>
constexpr bool is_array_like_v = is_range_loopable_v<T> && !is_associative_v<T>;

template <class T>
struct is_array_like : std::bool_constant<is_array_like_v<T>> {};

template <class T>
concept array_like = is_array_like_v<T>;


template <class T>
constexpr bool is_tuple_like_v = requires(T t) {
    std::tuple_size<std::remove_cvref_t<T>>::value;
    std::get<0>(t);
};
template <class T>
struct is_tuple_like : std::bool_constant<is_tuple_like_v<T>> {};

template <class T>
concept tuple_like = is_tuple_like_v<T>;


template <class T>
constexpr bool is_expected_v = requires(T e) {
    typename std::remove_cvref_t<T>::value_type;
    typename std::remove_cvref_t<T>::error_type;
    typename std::remove_cvref_t<T>::unexpected_type;
    e.has_value();
    e.error();
    e.value();
};
template <class T>
struct is_expected : std::bool_constant<is_expected_v<T>> {};

template <class T>
concept expectable = is_expected_v<T>;


template <class T>
constexpr bool is_optional_v = !expectable<T> && requires(T o) {
    o.value();
    o.has_value();
    o.operator*();
    typename std::remove_cvref_t<T>::value_type;
};
template <class T>
struct is_optional : std::bool_constant<is_optional_v<T>> {};

template <class T>
concept optionable = is_optional_v<T>;


template <class T>
constexpr bool is_character_v = is_one_of_v<std::remove_cv_t<T>, char, wchar_t, char8_t, char16_t, char32_t>;

template <class T>
struct is_character : std::bool_constant<is_character_v<T>> {};

template <class T>
concept character = is_character_v<T>;


template <class T>
constexpr bool is_non_char_integral_v = std::is_integral_v<T> && !is_character_v<T>;

template <class T>
struct is_non_char_integral : std::bool_constant<is_non_char_integral_v<T>> {};

template <class T>
concept non_char_integral = is_non_char_integral_v<T>;


template <class T>
constexpr bool is_constructible_to_string_v = std::is_constructible_v<std::string, T>;

template <class T>
struct is_constructible_to_string : std::bool_constant<is_constructible_to_string_v<T>> {};

template <class T>
concept constructible_to_string = is_constructible_to_string_v<T>;


template <auto V>
constexpr std::string_view name_of_impl() noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    constexpr std::string_view n{__FUNCSIG__};
    constexpr std::string_view k{"name_of_impl<"};
    constexpr std::string_view l{">(void) noexcept"};
#else
    constexpr std::string_view n{__PRETTY_FUNCTION__};
    constexpr std::string_view k{"[V = "};
    constexpr std::string_view l{"]"};
#endif
    constexpr auto p = n.find(k) + k.size();

    return n.substr(p, n.size() - p - l.size());
}
template <class T>
consteval std::string_view name_of_impl() noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    constexpr std::string_view n{__FUNCSIG__};
    constexpr std::string_view k{"name_of_impl<"};
    constexpr std::string_view l{">(void) noexcept"};
#else
    constexpr std::string_view n{__PRETTY_FUNCTION__};
    constexpr std::string_view k{"[T = "};
    constexpr std::string_view l{"]"};
#endif
    constexpr auto p = n.find(k) + k.size();

    return n.substr(p, n.size() - p - l.size());
}
template <class T>
constexpr std::string_view type_name_v = ::oeo::name_of_impl<T>();

template <auto V>
constexpr std::string_view nontype_name_v = ::oeo::name_of_impl<V>();


template <class T, T V1, T V2, class = std::bool_constant<true>>
struct is_comparable : std::false_type {};

template <class T, T V1, T V2>
struct is_comparable<T, V1, V2, std::bool_constant<V1 == V2>> : std::true_type {};

template <class T, T V1, T V2>
constexpr bool is_comparable_v = !is_comparable<T, V1, V2>::value;


template <class T>
constexpr bool is_struct_v = false;

template <class T, auto f>
constexpr bool is_virtual_function_pointer_v = false;

#if defined(_MSC_VER) && !defined(__clang__)

template <require<std::is_class> T>
constexpr bool is_struct_v<T> = type_name_v<T>.starts_with("struct ");

template <require<std::is_member_function_pointer> T, T f>
constexpr bool is_virtual_function_pointer_v<T, f> = nontype_name_v<f>.find("::`vcall'{") != std::string_view::npos;

#else

template <require<std::is_member_function_pointer> T, T f>
constexpr bool is_virtual_function_pointer_v<T, f> = !is_comparable_v<T, f, f>;

#endif

template <class T>
struct is_struct : std::bool_constant<is_struct_v<T>> {};

template <class T, auto f>
struct is_virtual_function_pointer : std::bool_constant<is_virtual_function_pointer_v<T, f>> {};

} // namespace oeo
