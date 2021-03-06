#pragma once

#include <Arduino.h>
#include <WString.h>

#include <chrono>
#include <vector>

inline String operator"" _str(const char* s, size_t) {
    return String(s);
}

template <typename T>
String ToString(T&& v) {
    String ret;
    ret += v;
    return ret;
}

inline String ToString(std::chrono::milliseconds ms) {
    return String(static_cast<long>(ms.count())) + " ms";
}

template <typename T>
inline String ToString(const std::vector<T>& data, String sep = ", ") {
    String ret;
    for (const auto& d : data) {
        ret += d;
        ret += sep;
    }
    if (ret)
        ret.remove(ret.length() - sep.length());
    return ret;
}

template <typename T>
StringSumHelper& operator+ (StringSumHelper &lhs, const std::vector<T>& data) {
    lhs.concat(ToString(data));
    return lhs;
}


inline const char* YesNo(bool condition) {
    return condition ? "yes" : "no";
}

inline const char* OnOff(bool condition) {
    return condition ? "on" : "off";
}

inline std::chrono::milliseconds BoardTimeDifference(std::chrono::milliseconds before, std::chrono::milliseconds now) {
    // I assume we store time at least once per 2³² ms, that is more than a month and a half.
    if (now < before)
        return std::chrono::milliseconds(std::numeric_limits<int>::max() - before.count() + now.count());
    else
        return now - before;
}

#if __cplusplus < 201402L || defined(IN_KDEVELOP_PARSER)
// before C++14
    namespace cxx_14 {
        template <size_t... i>
        struct index_sequence {};

        namespace internal {
            template <typename T>
            struct self { using type = T; };

            template <size_t n, size_t... is>
            struct make_index_sequence : public make_index_sequence<n - 1, n - 1, is...>
            {};

            template <size_t... is>
            struct make_index_sequence<0u, is...> {
                using type = index_sequence<is...>;
            };
        }

        template <size_t n>
        using make_index_sequence = typename internal::make_index_sequence<n>::type;

        using one_seq = make_index_sequence<1>;
    }

    namespace std {
        namespace chrono_literals {
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wliteral-suffix"  // might not work with old GCC as well :(
            constexpr std::chrono::milliseconds operator "" ms(unsigned long long value)
            {
                return std::chrono::milliseconds(value);
            }

            constexpr std::chrono::milliseconds operator "" s(unsigned long long value)
            {
                return std::chrono::seconds(value);
            }

            constexpr std::chrono::minutes operator "" min(unsigned long long value)
            {
                return std::chrono::minutes(value);
            }
            #pragma GCC diagnostic pop
        }

        using cxx_14::index_sequence;
        using cxx_14::make_index_sequence;
    }

#   define DEFAULT_VALUE_FOR_AGGREGATE_TYPE(x)
#else
// C++14 and later
#   define DEFAULT_VALUE_FOR_AGGREGATE_TYPE(x) = x
#endif  // pre-C++14

using namespace std::chrono_literals;

#if __cplusplus < 201703L
    namespace cxx_17 {
        namespace internal {
            template <class F, class Tuple, std::size_t... I>
            constexpr auto apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) -> decltype(f(std::get<I>(std::forward<Tuple>(t))...)) {
                return f(std::get<I>(std::forward<Tuple>(t))...);
            }
        }

        template <class F, class Tuple>
        constexpr auto apply(F&& f, Tuple&& t) -> decltype(internal::apply_impl(
                std::forward<F>(f), std::forward<Tuple>(t),
                std::make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>{}
            )) {
            return internal::apply_impl(
                std::forward<F>(f), std::forward<Tuple>(t),
                std::make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>{}
            );
        }
    }

    namespace std {
        using cxx_17::apply;
    }

#endif  // pre-c++17
