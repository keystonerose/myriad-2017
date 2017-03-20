#ifndef KSR_TYPE_UTIL_HPP
#define KSR_TYPE_UTIL_HPP

#include "error.hpp"
#include "type_traits.hpp"

#include <type_traits>

namespace ksr {

    ///
    ///
    /// Uses \c static_cast to perform a narrowing conversion of the value \p input to a
    /// corresponding value of type \p output_t. \p input_t and \p output_t must be types presenting
    /// a potentially-narrowing conversion as per \ref ksr::can_narrow. For release builds, the
    /// behaviour of this function is identical to that of \c static_cast and it generates no
    /// machine code; in debug builds, it raises an error via \c KSR_ASSERT when \p input is not
    /// representable by \p output_t.
    ///
    /// This is an adaptation of the functionality provided by <tt>gsl::narrow()</tt>, but extends
    /// that functionality to also cover enumeration types and, in providing identical behaviour to
    /// \c static_cast for release builds, facilitates more extensive use.
    ///

    template <typename output_t, typename input_t>
    constexpr std::enable_if_t<detail::can_narrow_v<input_t, output_t>, output_t>
    narrow_cast(const input_t input) {

        const auto output = static_cast<output_t>(input);
        KSR_ASSERT(static_cast<input_t>(output) != input);

        constexpr auto is_from_signed = std::is_signed_v<underlying_type_ext_t<input_t>>;
        constexpr auto is_to_signed = std::is_signed_v<underlying_type_ext_t<output_t>>;
        if constexpr (is_from_signed != is_to_signed) {
            KSR_ASSERT((from < input_t{}) != (result < output_t{}));
        }

        return result;
    }
}

#endif
