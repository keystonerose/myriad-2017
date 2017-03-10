#include "pairer.hpp"
#include "engine.hpp"

namespace myriad {

    int deduplicate_pairer::count() const {
        const auto size = m_set.size();
        return size * (size - 1) / 2;
    }

    void deduplicate_pairer::pair(const ksr::function_view<compare_sig> callback) {

        auto lhs_iter = std::begin(m_set);
        const auto end = std::end(m_set);
        for (; lhs_iter != end; ++lhs_iter) {

            auto rhs_iter = lhs_iter;
            ++rhs_iter;
            for (; rhs_iter != end && !thread_interrupted(); ++rhs_iter) {

                const auto choice = callback(*lhs_iter, *rhs_iter);
                switch (choice) {
                    case discard_choice::lhs:
                        lhs_iter = m_set.erase(lhs_iter);
                        break;

                    case discard_choice::rhs:
                        rhs_iter = m_set.erase(rhs_iter);
                        break;

                    case discard_choice::none:
                        break;
                }
            }
        }
    }

    int merge_pairer::count() const {
        return m_src_set.size() * m_dst_set.size();
    }

    void merge_pairer::pair(const ksr::function_view<compare_sig> callback) {

        auto src_iter = std::begin(m_src_set);
        const auto src_end = std::end(m_src_set);

        for (; src_iter != src_end; ++src_iter) {

            auto dst_iter = std::begin(m_dst_set);
            const auto dst_end = std::end(m_dst_set);

            for (; dst_iter != dst_end && !thread_interrupted(); ++dst_iter) {

                const auto choice = callback(*src_iter, *dst_iter);
                switch (choice) {
                    case discard_choice::lhs:
                        src_iter = m_src_set.erase(src_iter);
                        break;

                    case discard_choice::rhs:
                        dst_iter = m_dst_set.erase(dst_iter);
                        break;

                    case discard_choice::none:
                        break;
                }
            }
        }
    }
}
