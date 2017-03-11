#ifndef KSR_ERROR_HPP
#define KSR_ERROR_HPP

#include <stdexcept>

#define KSR_STRINGIFY_DETAIL(x) #x
#define KSR_STRINGIFY(x) KSR_STRINGIFY_DETAIL(x)

#ifdef NDEBUG
#define KSR_ASSERT(cond)
#else
#define KSR_ASSERT(cond) \
    if (!(cond)) throw ksr::logic_error{__FILE__ ":" KSR_STRINGIFY(__LINE__) ": assertion '" #cond "' failed"};
#endif

namespace ksr {
    struct logic_error : std::logic_error {
        using std::logic_error::logic_error;
    };
}

#undef KSR_STRINGIFY
#undef KSR_STRINGIFY_DETAIL

#endif
