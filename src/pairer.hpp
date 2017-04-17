#ifndef MYRIAD_PAIRER_HPP
#define MYRIAD_PAIRER_HPP

#include "hash.hpp"
#include "image_info.hpp"
#include "ksr/function_view.hpp"

#include <unordered_set>

namespace myriad {

    // We need our image_info container to provide reasonable (optimised) performance on erasure
    // and search, robust iterator invalidation guarantees make life much nicer in
    // compare_images(), and the cases where contiguous-memory performance gains kick in are
    // those cases where the container is sufficiently small that its performance doesn't really
    // matter. So std::unordered_set is a good choice for now (though std::colony would be worth
    // a look if it ever becomes a thing; not worth the extra dependency for now though).

    using image_set = std::unordered_set<image_info>;

    enum class discard_choice { none, lhs, rhs };

    // TODO We don't need the virtual function mechanism (and therefore function_view); a
    // templatised approach would be better.

    ///
    /// \ref pairer implementations provide specific algorithms for matching up images within one or
    /// more containers, defining which images get compared to which other images and the order in
    /// which these comparisons happen.
    ///

    class pairer {
    public:

        using compare_sig = discard_choice(const image_info&, const image_info&);

        ///
        /// Calculates the number of image pairings that will be processed by the \ref pairer
        /// implementation, when the pair() member function is executed to completion.
        ///

        virtual int count() const = 0;

        ///
        /// Matches up \ref image_info objects from within a container or containers referenced by
        /// the specific \ref pairer implementation, and for each such pair, calls \p callback to
        /// determine which of the pair (if either) should be discarded, then erases elements from
        /// or moves elements between the underlying containers if appropriate before continuing.
        /// This process may be interrupted by requesting an interruption on the calling thread.
        ///

        virtual void pair(ksr::function_view<compare_sig> callback) = 0;
    };

    ///
    /// Pairs every \ref image_info object within a single container with every other such object
    /// exactly once, in an unspecified order. If the the callback passed to pair() specifies that
    /// one of the paired images should be discarded, it is deleted without any effect on the other
    /// image in the pair.
    ///

    class deduplicate_pairer : public pairer {
    public:

        explicit deduplicate_pairer(image_set& set)
          : m_set{set} {
        }

        int count() const override;
        void pair(ksr::function_view<compare_sig> callback) override;

    private:
        image_set& m_set;
    };

    ///
    /// Pairs each \ref image_info object in a source container with every \ref image_info object in
    /// a destination container (but does not compare images internally within either of these
    /// containers). If the callback passed to pair() specifies that one of the paired images should
    /// be discarded, the other image in the pair is moved to its former location following the
    /// deletion. This callback is passed the source image as its first argument and the destination
    /// image as the second argument.
    ///

    class merge_pairer : public pairer {
    public:

        explicit merge_pairer(image_set& src_set, image_set& dst_set)
          : m_src_set{src_set}, m_dst_set{dst_set} {
        }

        int count() const override;
        void pair(ksr::function_view<compare_sig> callback) override;

    private:
        image_set &m_src_set;
        image_set &m_dst_set;
    };
}

#endif
