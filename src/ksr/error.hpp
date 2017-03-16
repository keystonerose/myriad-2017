#ifndef KSR_ERROR_HPP
#define KSR_ERROR_HPP

#include <sstream>
#include <stdexcept>

#define KSR_STRINGIFY_DETAIL(x) #x
#define KSR_STRINGIFY(x) KSR_STRINGIFY_DETAIL(x)

#ifdef NDEBUG
#define KSR_ASSERT(cond) (void)sizeof(cond)
#else
#define KSR_ASSERT(cond) ksr::detail::debug_assert((cond), #cond, __function__, __FILE__, __LINE__)
#endif

namespace ksr {

    struct logic_error : std::logic_error {
        using std::logic_error::logic_error;
    };

    namespace detail {

        ///
        /// Implementation of the KSR_ASSERT macro when compiling in debug mode. Raises an error
        /// if \p condition is false.
        ///

        inline void debug_assert(
            const bool cond, const char* const cond_str,
            const char* const function, const char* const file, const int line) {

            if (cond) {
                return;
            }

            std::stringstream ss;



            if (!condition) {

                  __FUNCTION__ "@" __FILE__ ":" KSR_STRINGIFY(__LINE__) ": assertion '" #cond "' failed"

                throw ksr::logic_error{message};
            }
        }
    }
}

#endif
