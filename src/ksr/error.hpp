#ifndef KSR_ERROR_HPP
#define KSR_ERROR_HPP

#include <cassert>
#include <sstream>
#include <stdexcept>

///
/// If \c KSR_THROW_ON_ASSERT is defined, the macro \ref KSR_ASSERT will cause an exception of type
/// \ref ksr::logic_error to be thrown on failure; otherwise, it will have no effect (but not raise
/// an unused-variable warning) if \c NDEBUG is defined, or evaluate to the standard \ref assert
/// otherwise.
///

#if defined(KSR_THROW_ON_ASSERT)
#define KSR_ASSERT(cond) ksr::detail::throw_assert((cond), #cond, __func__, __FILE__, __LINE__)
#elif defined(NDEBUG)
#define KSR_ASSERT(cond) (void)sizeof(cond)
#else
#define KSR_ASSERT(cond) assert(cond)
#endif

namespace ksr {

    struct logic_error : std::logic_error {
        using std::logic_error::logic_error;
    };

    namespace detail {

        ///
        /// Implementation of the KSR_ASSERT macro when compiling in debug mode with
        /// KSR_THROW_ON_ASSERT defined. Throws a \c ksr::logic_error with information about the
        /// location at which the error occurred if \p cond is false; otherwise does nothing.
        ///

        inline void throw_assert(
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
