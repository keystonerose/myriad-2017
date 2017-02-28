#ifndef KSR_DENSE_UPDATER_HPP
#define KSR_DENSE_UPDATER_HPP

#include <chrono>
#include <utility>

namespace ksr {

    ///
    /// Specifies whether an update in a dense sequence represents an intermediate state of the
    /// object being updated (in which case the type is \c transient) or its final state (in which
    /// case the type in \c final and some significant amount of time is expected to elapsed before
    /// another such update is performed).
    ///

    enum class dense_update_type { transient, final };

    ///
    /// Provides an interface for conditionally executing function objects if sufficient time has
    /// elapsed since the last such execution. This may be useful when signalling a rapid sequence
    /// of changes where the exact transient states of the object being described are not critically
    /// important and there is a limitation upon how frequently these transient states may be acted
    /// upon.
    ///
    /// \p Duration must be an instantiation of \c std::chrono::duration; a duration of this type
    /// must then be passed to the \ref dense_updater constructor to specify the permissible
    /// interval between successive updates.
    ///

    template <typename Duration>
    class dense_updater;

    template <typename IntervalRep, typename IntervalPeriod>
    class dense_updater<std::chrono::duration<IntervalRep, IntervalPeriod>> {
    public:

        using duration = std::chrono::duration<IntervalRep, IntervalPeriod>;
        explicit dense_updater(const duration interval)
          : m_interval{interval} {}

        ///
        /// Executes the callback \p update with arguments \p args if \p type is \c final, or else
        /// if the elapsed time since try_update() was last called is greater than the duration that
        /// the \ref dense_updater was constructed with. \p UpdateFunction must be a function type
        /// returning \c void and taking the arguments as specified by the \p Args pack.
        ///

        template <typename UpdateFunction, typename... Args>
        bool try_update(const dense_update_type type, UpdateFunction update, Args&&... args) const {

            const auto now = std::chrono::steady_clock::now();
            const auto elapsed = now - m_last_update_time;

            // If this is the first time try_update() has been called, m_last_update_time will be set to
            // the epoch of std::chrono::steady_clock, so in the absence of perverse interval choices,
            // the elapsed time will be large enough to ensure that m_update gets called.

            if (type == dense_update_type::final || elapsed >= m_interval) {
                update(std::forward<Args>(args)...);
                m_last_update_time = std::chrono::steady_clock::now();
                return true;
            }

            return false;
        }

    private:

        duration m_interval;
        mutable std::chrono::steady_clock::time_point m_last_update_time;
    };
}

#endif

