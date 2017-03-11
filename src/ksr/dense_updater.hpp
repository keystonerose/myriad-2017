#ifndef KSR_DENSE_UPDATER_HPP
#define KSR_DENSE_UPDATER_HPP

#include <chrono>
#include <cstddef>
#include <numeric>
#include <tuple>
#include <utility>

namespace ksr {

    namespace detail {

        template <typename Duration, typename... ItemTypes>
        class dense_update_policy;

        template <typename IntervalRep, typename IntervalPeriod>
        class dense_update_policy<std::chrono::duration<IntervalRep, IntervalPeriod>> {
        protected:

            using duration = std::chrono::duration<IntervalRep, IntervalPeriod>;
            explicit dense_accumulator(const duration interval)
              : m_interval{interval} {}

            bool needs_update() {

                const auto now = std::chrono::steady_clock::now();
                const auto elapsed = now - m_last_update_time;

                // If this is the first time needs_update() has been called, m_last_update_time will
                // be set to the epoch of std::chrono::steady_clock, so in the absence of perverse
                // interval choices, the elapsed time will be large enough to ensure that m_update
                // gets called.

                if (type == dense_update_type::final || elapsed >= m_interval) {
                    m_last_update_time = std::chrono::steady_clock::now();
                    return true;
                }

                return false;
            }

        private:

            duration m_interval;
            mutable std::chrono::steady_clock::time_point m_last_update_time;
        };

        class percentage_update_policy {
        protected:

            explicit percentage_update_policy(const int complete, const int total)
              : m_last_percent_complete{int_percentage(complete, total)} {}

            template <typename UpdateFunction, typename... Args>
            bool try_update(const int complete, const int total, UpdateFunction update, Args&&... args) const {

            }

        private:

            static int int_percentage(const int num, const int denom) {
                const auto result = std::lround(100.0f * static_cast<float>(num) / static_cast<float>(denom));
                return gsl::narrow_cast<int>(result);
            }

            mutable int m_last_percent_complete;
        };

        template <typename UpdatePolicy, typename UpdateFunction, typename... Types>
        class accumulator : private UpdatePolicy {
        public:

            explicit accumulator(UpdateFunction update, Types&&... initial_values)
              : m_update{std::forward<UpdateFunction>(update)},
                m_values{std::forward<Types>(initial_values)...} {}

            template <std::size_t Index>
            decltype(auto) get() const {
                return std::get<Index>(m_values);
            }

            void force_update(Types&&... new_values) const {
                m_update(new_values);
                m_values = std::make_tuple(std::forward<Types>(new_values)...);
            }

            bool update(Types&&... new_values) const {

                const auto needs_update = UpdatePolicy::needs_update(new_values...);
                if (needs_update) {
                    m_update(new_values);
                }

                m_values = std::make_tuple(std::forward<Types>(new_values)...);
                return needs_update;
            }

        private:

            UpdateFunction m_update;
            std::tuple<Types...> m_values;
        };
    }

    ///
    /// Specifies whether an update in a dense sequence represents an intermediate state of the
    /// object being updated (in which case the type is \c transient) or its final state (in which
    /// case the type in \c final and some significant amount of time is expected to elapsed before
    /// another such update is performed).
    ///

    enum class dense_update_type { transient, final };

    // TODO:DOC
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

    template <typename Duration, typename... ItemTypes>
    class dense_accumulator;

    template <typename IntervalRep, typename IntervalPeriod, typename... ItemTypes>
    class dense_accumulator<std::chrono::duration<IntervalRep, IntervalPeriod>, ItemTypes...> {
    public:

        using duration = std::chrono::duration<IntervalRep, IntervalPeriod>;
        explicit dense_accumulator(const duration interval, ItemTypes&&... items)
          : m_interval{interval}, m_items{std::forward<ItemTypes>(items)...} {}

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
        std::tuple<ItemTypes...> m_items;

        mutable std::chrono::steady_clock::time_point m_last_update_time;
    };
}

namespace std {

    // TODO Test structured binding integration

    template<std::size_t Index, typename UpdatePolicy, typename... Types>
    struct tuple_element<Index, ksr::detail::accumulator<UpdatePolicy, Types...>> {
        using type = decltype(std::declval<ksr::detail::accumulator>().get<Index>());
    };

    template <typename UpdatePolicy, typename... Types>
    struct tuple_size<ksr::detail::accumulator<UpdatePolicy, Type...>>
        : public std::integral_constant<std::size_t, sizeof...(Types)> {};
}

#endif

