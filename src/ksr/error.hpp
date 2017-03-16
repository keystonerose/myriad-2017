#ifndef KSR_ERROR_HPP
#define KSR_ERROR_HPP

#include <sstream>
#include <stdexcept>

#ifdef NDEBUG
#define KSR_ASSERT(cond) (void)sizeof(cond)
#else
#define KSR_ASSERT(cond) ksr::detail::debug_assert((cond), #cond, __func__, __FILE__, __LINE__)
#endif

namespace ksr {

    struct logic_error : std::logic_error {
        using std::logic_error::logic_error;
    };

    namespace detail {

        ///
        /// Implementation of the KSR_ASSERT macro when compiling in debug mode. Raises an error
        /// with information about the location at which the error occurred if \p cond is false;
        /// otherwise does nothing.
        ///

        inline void debug_assert(
            const bool cond, const char* const cond_str,
            const char* const function, const char* const file, const int line) {

            if (cond) {
                return;
            }

            std::ostringstream buffer;
            buffer << function << "@" << file << ":" << line << ": ";
            buffer << "assertion `" << cond_str << "` failed";
            throw ksr::logic_error{buffer.str()};
        }
    }
}

#endif
