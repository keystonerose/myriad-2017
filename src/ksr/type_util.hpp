#ifndef KSR_TYPE_UTIL_HPP
#define KSR_TYPE_UTIL_HPP

#include <cassert>
#include <type_traits>

namespace ksr {

    namespace type_tags {

        ///
        /// Wraps a single type \p T in an object that can be passed to or returned from
        /// \c constexpr functions performing compile-time operations on types. The tagged type can
        /// be retrieved from a \ref tag object using \c decltype and the (undefined) member
        /// function get().
        ///

        template <typename T>
        struct tag {
            static constexpr T get();
        };

        ///
        /// An extension of \c std::underlying_type that is applicable to arbitrary types. If \p T
        /// is an enumeration type, the returned type (tag) corresponds to that specified by
        /// \c std::underlying_type; otherwise, \p T itself is returned. This function is wrapped by
        /// the alias \c underlying_type_t, which is expressed in terms of types rather than type
        /// tags but otherwise has identical behaviour.
        ///

        template <typename T>
        constexpr auto underlying_type_ext(tag<T>) {
            if constexpr (std::is_enum_v<T>) {
                return tag<std::underlying_type_t<T>>{};
            } else {
                return tag<T>{};
            }
        }
    }

    template <typename T>
    using underlying_type_ext_t = decltype(type_tags::underlying_type_ext(type_tags::tag<T>{}).get());

    ///
    /// Uses \c static_cast to perform a narrowing conversion of the value \p from to a
    /// corresponding value of type \p To. \p From and \p To must be arithmetic or enumeration
    /// types. For release builds, the behaviour of this function is identical to that of
    /// \c static_cast and it generates no machine code; in debug builds, it asserts when the
    /// narrowing conversion caused information to be lost.
    ///
    /// This is an adaptation of the functionality provided by <tt>gsl::narrow()</tt>, but extends
    /// that functionality to also cover enumeration types and, in providing identical behaviour to
    /// \c static_cast for release builds, facilitates more extensive use.
    ///

    template <typename To, typename From>
    constexpr To narrow_cast(const From from) noexcept {

        static_assert(std::is_arithmetic_v<From> || std::is_enum_v<From>);
        static_assert(std::is_arithmetic_v<To> || std::is_enum_v<To>);

        const auto result = static_cast<To>(from);
        assert(static_cast<From>(result) != from);

        constexpr auto is_from_signed = std::is_signed_v<underlying_type_ext_t<From>>;
        constexpr auto is_to_signed = std::is_signed_v<underlying_type_ext_t<To>>;
        if constexpr (is_from_signed != is_to_signed) {
            assert((from < From{}) != (result < To{}));
        }

        return result;
    }
}

#endif
