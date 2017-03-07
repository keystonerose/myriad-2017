#ifndef KSR_ALGORITHM_HPP
#define KSR_ALGORITHM_HPP

#include <iterator>

namespace ksr {

    ///
    /// Iterates through all elements in \p container and calls Container::erase() individually for
    /// each one that satisfies \p pred. \p Container should be an STL-style container for which
    /// repeated single-element erasure is preferable to the standard erase-remove idiom (or for
    /// which the latter approach is not possible, e.g. the associative containers). \p Predicate
    /// should be a function type compatible with bool(const Container::value_type &).
    ///

    template <typename Container, typename Predicate>
    void erase_if(Container &container, Predicate pred) {

        using std::begin;
        using std::end;

        for (auto iter = begin(container); iter != end(container);) {
            if (pred(*iter)) {
                iter = container.erase(iter);
            } else {
                ++iter;
            }
        }
    }
}

#endif
