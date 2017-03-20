#ifndef KSR_TYPE_TRAITS_HPP
#define KSR_TYPE_TRAITS_HPP

#include <type_traits>

// TODO:DOC the type_tags

namespace ksr {

    namespace type_tags {

        ///
        /// Wraps a single type \p t in an object that can be passed to or returned from
        /// \c constexpr functions performing compile-time operations on types. The tagged type can
        /// be retrieved from a \ref tag object using \c decltype.
        ///

        template <typename t>
        struct tag {
            using type = t;
        };

        template <typename lhs_t, typename rhs_t>
        constexpr auto operator==(tag<lhs_t>, tag<rhs_t>) -> bool {
            return std::is_same_v<lhs_t, rhs_t>;
        }

        template <typename lhs_t, typename rhs_t>
        constexpr auto operator!=(const tag<lhs_t> lhs, const tag<rhs_t> rhs) -> bool {
            return !(lhs == rhs);
        }

        // TODO:DOC

        template <typename...>
        struct type_seq {

            template <typename t>
            constexpr auto contains(tag<t>) -> bool { return false; }

            template <typename lhs_t, typename rhs_t>
            constexpr auto less(tag<lhs_t>, tag<rhs_t>) -> bool { return false; }
        };

        template <typename head_t, typename... tail_ts>
        struct type_seq<head_t, tail_ts...> {

            constexpr auto head() -> tag<head_t> { return {}; }
            constexpr auto tail() -> type_seq<tail_ts...> { return {}; }

            // TODO:DOC

            template <typename t>
            constexpr auto contains(const tag<t> needle) -> bool {

                if constexpr (head() == needle) {
                    return true;
                } else {
                    return contains(tail(), needle);
                }
            }

            // TODO:DOC

            template <typename lhs_t, typename rhs_t>
            constexpr auto less(const tag<lhs_t> lhs, const tag<rhs_t> rhs) -> bool {

                if constexpr (head() == lhs) {
                    return seq_contains(tail(), rhs);
                } else {
                    return seq_less(tail(), lhs, rhs);
                }
            }
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

        template <typename t>
        constexpr auto underlying_type_ext(tag<t>) {
            if constexpr (std::is_enum_v<t>) {
                return tag<std::underlying_type_t<t>>{};
            } else {
                return tag<std::remove_cv_t<t>>{};
            }
        }
    }

    template <typename t>
    using underlying_type_ext = decltype(type_tags::underlying_type_ext(type_tags::tag<t>{}));

    template <typename t>
    using underlying_type_ext_t = typename underlying_type_ext<t>::type;

    namespace type_tags {

        // TODO:DOC
        // TODO:HERE

        template <typename output_t, typename input_t>
        constexpr auto can_narrow(tag<output_t>, tag<input_t>) -> bool {

            if constexpr (std::is_floating_point_v<output_t> && std::is_floating_point_v<input_t>) {

                constexpr auto floating_point_ranks = type_seq<float, double, long double>{};
                return floating_point_ranks.less(tag<output_t>{}, tag<input_t>{});

            } else {

                using underlying_output_t = underlying_type_ext_t<output_t>;
                using underlying_input_t = underlying_type_ext_t<input_t>;

                if constexpr (std::is_integral_v<underlying_output_t> && std::is_integral_v<underlying_input_t>) {

                    constexpr auto is_output_signed = std::is_signed_v<underlying_output_t>;
                    constexpr auto is_input_signed = std::is_signed_v<underlying_input_t>;
                    return (is_output_signed != is_input_signed) ||

                    // TODO:HERE

                } else {
                    return false;
                }
            }
        }
    }

    template <typename output_t, typename input_t>
    struct can_narrow : integral_constant<
        type_tags::can_narrow(type_tags::tag<output_t>{}, type_tags::tag<input_t>{})> {};

    template <typename output_t, typename input_t>
    inline constexpr auto can_narrow_v = can_narrow<output_t, input_t>::value;
}

#endif
