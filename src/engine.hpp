#ifndef MYRIAD_ENGINE_HPP
#define MYRIAD_ENGINE_HPP

#include "image_info.hpp"
#include "pairer.hpp"

#include "ksr/meta.hpp"
#include "ksr/stdx/variant.hpp"
#include "ksr/update_filter.hpp"

#include <QObject>
#include <QString>
#include <QStringList>

namespace myriad {

    class engine;

    /// Enumerates the processing states that the `engine` passes through during a call to
    /// `merge()`; refer to the documentation for that function for more details.

    enum class phase { scan, hash, compare };

    using phase_range = ksr::meta::value_seq<
        phase::scan,
        phase::hash,
        phase::compare>;

    // TODO:DOC
    // In all phase_data constructors, the engine instance passed must outlive the phase_data object

    template <phase>
    struct phase_data;

    template <>
    struct phase_data<phase::scan> {
        explicit phase_data(const engine& self);
        ksr::sampled_filter<int, int> signaller;
    };

    template <>
    struct phase_data<phase::hash> {
        explicit phase_data(const engine& self);
        ksr::int_percentage_filter<int> signaller;
    };

    template <>
    struct phase_data<phase::compare> {
        explicit phase_data(const engine& self);
        ksr::int_percentage_filter<int> signaller;
    };

    using phase_variant
        = ksr::stdx::opt_variant_of<ksr::meta::transform_values_t<phase_range, phase_data>>;

    /// A stateless class providing member functions that may be invoked from a different thread to
    /// perform the fundamental processing provided by Myriad. The primary entry point is the
    /// `merge()` member function; refer to the documentation for that function for more details.

    class engine : public QObject {
        Q_OBJECT

    public:

        /// Merges a list of new image files with a "collection" of existing ones by identifying
        /// those new images that are visual duplicates of ones already in the collection and
        /// appraising whether to overwrite the existing version of the image with the new one. This
        /// "collection" is identified as the set of all image files that are descendants of the
        /// filesystem path `collection_path`. The process has three phases:
        ///
        /// 1. The collection is scanned to identify all existing image files.
        /// 2. All images, both those specified as inputs and those already in the collection, are
        ///    examined and their perceptual hashes are computed.
        /// 3. All images in the collection are compared with each other to identify duplicates;
        ///    when non-trivial duplicates are found, the `engine` pauses and until the user
        ///    indicates which version of the duplicated image should be kept.
        /// 4. Each input image is considered in turn, and compared with each existing image in the
        ///    collection. If it is a duplicate of an existing image, the input image is either
        ///    discarded or used as a replacement for that existing image, depending upon the
        ///    outcome of a decision as in (3); otherwise, no action is taken.
        ///
        /// The process may be interrupted at any point by requesting an interruption on the
        /// engine's thread. If `input_image_paths` contains paths that are descendants of
        /// `collection_path`, they are treated as part of the collection and not as inputs.

        Q_INVOKABLE
        void merge(const QStringList& input_image_paths, const QString& collection_path) const;

    Q_SIGNALS:

        void input_count_changed(int file_count, int folder_count) const;
        void phase_changed(phase new_phase) const;
        void progress_changed(int percent_complete) const;

    private:

        /// Changes the phase data object stored in `m_data` to be a new instance of
        /// `phase_data<new_phase>`, emits the `phase_changed()` signal accordingly, and
        /// additionally emits a `progress_changed()` signal to reset the current progress to zero
        /// accordingly. Returns a reference to the new `phase_data` object emplaced within
        /// `m_data`.

        template <phase new_phase>
        auto change_phase() const -> phase_data<new_phase>&;

        /// Compares images as specified by `pairer` (which should be of one of the classes defined
        /// in `pairer.hpp`), emitting the progress_changed() signal to indicate how close to
        /// completion this process is. A single merge operation may use a sequence of different
        /// pairer strategies; because of this, a call to compare_images() need not take the emitted
        /// progress from 0 to 100. `start_count` specifies how many images have already been
        /// compared before this particular call to compare_images() was made; `total_count`
        /// specifies how many images need to be compared before that phase of the merge operation
        /// is considered complete. This operation may be interrupted by requesting an interruption
        /// on the engine's thread.

        template <typename pairer_t>
        void compare_images(pairer_t& pairer) const;

        /// Gets the `phase_data` object associated with the engine's current execution phase, which
        /// must be known to be `current_phase`.

        template <phase current_phase>
        auto current_data() const -> phase_data<current_phase>&;

        /// Constructs an `image_info` object for each filesystem path in `paths`, emitting the
        /// `progress_changed()` signal to indicate how close to completion this process is. A
        /// single merge operation may hash more than one group of image paths; because of this, a
        /// call to `hash_images()` need not take the emitted percentage progress from 0 to 100.
        /// `start_count` specifies how many images have already been hashed before this particular
        /// call to `hash_images()` was made; `total_count` specifies how many images need to be
        /// hashed before that phase of the merge operation is considered complete. This operation
        /// may be interrupted by requesting an interruption on the engine's thread.

        auto hash_images(const QStringList &paths) const -> image_set;

        /// If `base_path` is the filesystem path to a directory, recursively scans the descendants
        /// of that directory and appends to `image_paths` the paths of all supported image files
        /// therein; if `base_path` is the path to a file, that file is itself appended to
        /// `image_paths` if it is supported. `folder_count` is set to the number of directories
        /// that were scanned during this operation. As this scan is performed, the
        /// `input_count_changed()` signal is emitted to indicate how many supported image files
        /// have been found and how many directories have been scanned. This operation may be
        /// interrupted by requesting an interruption on the engine's thread.

        void scan_for_images(
            const QString &base_path, QStringList &image_paths, int &folder_count) const;

        mutable phase_variant m_data;
    };

    auto thread_interrupted() -> bool;
}

#endif
