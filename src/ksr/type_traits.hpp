#ifndef KSR_TYPE_TRAITS_HPP
#define KSR_TYPE_TRAITS_HPP

#include <type_traits>

// TODO:DOC the type_tags

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
            using type = T;
        };
    }

    template <auto value>
    struct integral_constant : std::integral_constant<decltype(value), value> {};

    namespace type_tags {

        ///
        /// An extension of \c std::underlying_type that is applicable to arbitrary types. If \p T
        /// is an enumeration type, the returned type (tag) corresponds to that specified by
        /// \c std::underlying_type; otherwise, \p T itself is returned, with any CV qualifiers
        /// removed (for consistency with former case). This function is wrapped by the alias
        /// \c ksr::underlying_type_ext_t, which is expressed in terms of types rather than type
        /// tags but otherwise has identical behaviour.
        ///

        template <typename T>
        constexpr auto underlying_type_ext(tag<T>) {
            if constexpr (std::is_enum_v<T>) {
                return tag<std::underlying_type_t<T>>{};
            } else {
                return tag<std::remove_cv_t<T>>{};
            }
        }
    }

    template <typename T>
    using underlying_type_ext = decltype(type_tags::underlying_type_ext(type_tags::tag<T>{}));

    template <typename T>
    using underlying_type_ext_t = typename underlying_type_ext<T>::type;

    namespace type_tags {

        // TODO:DOC
        // TODO:HERE

        template <typename OutputType, typename InputType>
        constexpr auto can_narrow(tag<OutputType>, tag<InputType>) -> bool {

            using underlying_output_t = underlying_type_ext_t<OutputType>;
            using underlying_input_t = underlying_type_ext_t<InputType>;

            if constexpr (std::is_integral_v) {

            } else {
            }
        }
    }

    template <typename OutputType, typename InputType>
    struct can_narrow : integral_constant<
        type_tags::can_narrow(type_tags::tag<OutputType>{}, type_tags::tag<InputType>{})> {};

    template <typename OutputType, typename InputType>
    inline constexpr auto can_narrow_v = can_narrow<OutputType, InputType>::value;
}

#endif
