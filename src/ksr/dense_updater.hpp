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
    /// Stores a function object and provides an interface for conditionally executing it if
    /// sufficient time has elapsed since the last such execution. This may be useful when
    /// signalling a rapid sequence of changes where the exact transient states of the object being
    /// described are not critically important and there is a limitation upon how frequently these
    /// transient states may be acted upon. Should typically be constructed via a call to
    /// make_dense_updater().
    ///
    /// \c UpdateFunction should be a callable type returning \c void, which is permitted to be
    /// mutable. It may take parameters of any number and type; these are forwarded to the stored
    /// callback when try_update() is called.
    ///

    template <typename IntervalRep, typename IntervalPeriod, typename UpdateFunction>
    class dense_updater {
    public:

        explicit dense_updater(
            const std::chrono::duration<IntervalRep, IntervalPeriod> interval,
            UpdateFunction update)
          : m_interval{interval}, m_update{std::move(update)} {}
        
        ///
        /// Executes the callback stored within the \ref dense_updater if \p type is \c final, or
        /// else if the elapsed time since try_update() was last called is greater than the duration
        /// that the \ref dense_updater was constructed with.
        ///
        
        template <typename... Args>
        bool try_update(dense_update_type type, Args&&... args);

    private:
        
        std::chrono::duration<IntervalRep, IntervalPeriod> m_interval;
        UpdateFunction m_update;

        mutable std::chrono::steady_clock::time_point m_last_update_time;
    };
    
    template <typename IntervalRep, typename IntervalPeriod, typename UpdateFunction>
    template <typename... Args>
    bool dense_updater<IntervalRep, IntervalPeriod, UpdateFunction>::try_update(
        const dense_update_type type, Args&&... args) {

        const auto now = std::chrono::steady_clock::now();
        const auto elapsed = now - m_last_update_time;

        // If this is the first time try_update() has been called, m_last_update_time will be set to
        // the epoch of std::chrono::steady_clock, so in the absence of perverse interval choices,
        // the elapsed time will be large enough to ensure that m_update gets called.

        if (type == dense_update_type::final || elapsed >= m_interval) {
            m_update(std::forward<Args>(args)...);
            m_last_update_time = std::chrono::steady_clock::now();
            return true;
        }
        
        return false;
    }
    
    template <typename IntervalRep, typename IntervalPeriod, typename UpdateFunction>
    auto make_dense_updater(
        const std::chrono::duration<IntervalRep, IntervalPeriod> interval, UpdateFunction update) {

        return dense_updater<IntervalRep, IntervalPeriod, UpdateFunction>{
            interval, std::move(update)};
    }
}

#endif

