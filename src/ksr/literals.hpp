#ifndef KSR_LITERALS_HPP
#define KSR_LITERALS_HPP

#include "meta.hpp"

#include <cstdint>
#include <limits>

namespace ksr::literals {

    namespace detail {

        constexpr unsigned int digit_from_char(const char digit_char) {

            if constexpr ('0' <= digit_char && digit_char <= '9') {
                return digit_char - '0';
            } else if ('A' <= digit_char && digit_char <= 'F') {
                return 0xA + (digit_char - 'A');
            } else if ('a' <= digit_char && digit_char <= 'f') {
                return 0xa + (digit_char - 'a');
            } else {
                static_assert(false);
            }
        }

        template <char head, char... tail>
        constexpr unsigned long long parse(const unsigned int base) {
        }

        template <char head, char... tail>
        constexpr unsigned long long parse_alt_base() {

            if constexpr (head == 'b' || head == 'B') {
                return parse<tail...>(2);
            } else if (head == 'x' || head == 'X') {
                return parse<tail...>(16);
            } else {
                return parse<tail...>(8);
            }
        }

        template <char head, char... tail>
        constexpr unsigned long long parse() {

            if constexpr (head == '0') {
                return parse_alt_base<tail...>();
            } else {
                return parse<head, tail...>(10);
            }
        }
    }

    template <char... digits>
    constexpr std::uint8_t operator "" _u8() {

        constexpr auto value = parse<digits...>();

        constexpr auto too_low =

        static_assert(std::numeric_limits<std::uint8_t>::min() <= value && value <= std::numeric_limits<std::uint8_t>::max();
    }

    decimal-literal ud-suffix	(1)
octal-literal ud-suffix	(2)
hex-literal ud-suffix	(3)
binary-literal ud-suffix

    unsigned long long int
}

#endif
