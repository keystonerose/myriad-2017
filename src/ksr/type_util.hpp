#ifndef KSR_TYPE_UTIL_HPP
#define KSR_TYPE_UTIL_HPP

#include "error.hpp"
#include "type_traits.hpp"

#include <type_traits>

namespace ksr {

    ///
    /// Uses \c static_cast to perform a narrowing conversion of the value \p input to a
    /// corresponding value of type \p OutputType. \p InputType and \p OutputType must be integral
    /// or enumeration types. For release builds, the behaviour of this function is identical to
    /// that of \c static_cast and it generates no machine code; in debug builds, it asserts when
    /// \p input is not representable by \p OutputType.
    ///

    template <typename OutputType, typename InputType>
    constexpr std::enable_if_t<detail::can_narrow_v<InputType, OutputType>, OutputType>
    narrow_cast(const InputType input) {

        // TODO:HERE

        // Hmmm
        // see http://stackoverflow.com/questions/17860657/well-defined-narrowing-cast

        using underlying_input_t = underlying_type_ext_t<InputType>;
        using underlying_output_t = underlying_type_ext_t<OutputType>;

        static_assert(std::is_arithmetic_v<underlying_input_t>);
        static_assert(std::is_arithmetic_v<underlying_output_t>);

        const auto output = static_cast<OutputType>(input);

        if constexpr (std::is_integral_v<InputType> &&

        KSR_ASSERT(static_cast<InputType>(output) == input);
        KSR_ASSERT((input < InputType{}) == (output < OutputType{}));

        /*
        // TODO:remove? Maybe not, since we'll need to convert to underlying type when supporting
        // enums

        constexpr auto is_input_signed = std::is_signed_v<underlying_type_ext_t<InputType>>;
        constexpr auto is_output_signed = std::is_signed_v<underlying_type_ext_t<OutputType>>;
        if constexpr (is_input_signed != is_output_signed) {
            KSR_ASSERT((input < InputType{}) == (output < OutputType{}));
        }
        */

        return output;
    }
}

#endif
