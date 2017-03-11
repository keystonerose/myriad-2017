#ifndef MYRIAD_ENGINE_HPP
#define MYRIAD_ENGINE_HPP

#include "image_info.hpp"
#include "pairer.hpp"
#include "ksr/dense_updater.hpp"

#include <QObject>
#include <QStringList>
#include <unordered_set>

class QString;

namespace myriad {

    ///
    /// A stateless class providing member functions that may be invoked from a different thread to
    /// perform the fundamental processing provided by Myriad. The primary entry point is the
    /// merge() member function; refer to the documentation for that function for more details.
    ///

    class engine : public QObject {
        Q_OBJECT

    public:

        ///
        /// Enumerates the processing states that the \ref engine passes through during a call to
        /// merge(); refer to the documentation for that function for more details.
        ///

        enum class phase { scan, hash, compare };

        explicit engine();

        ///
        /// Merges a list of new image files with a "collection" of existing ones by identifying
        /// those new images that are visual duplicates of ones already in the collection and
        /// appraising whether to overwrite the existing version of the image with the new one. This
        /// "collection" is identified as the set of all image files that are descendants of the
        /// filesystem path \p collection_path. The process has three phases:
        ///
        /// 1. The collection is scanned to identify all existing image files.
        /// 2. All images, both those specified as inputs and those already in the collection, are
        ///    examined and their perceptual hashes are computed.
        /// 3. All images in the collection are compared with each other to identify duplicates;
        ///    when non-trivial duplicates are found, the \ref engine pauses and until the user
        ///    indicates which version of the duplicated image should be kept.
        /// 4. Each input image is considered in turn, and compared with each existing image in the
        ///    collection. If it is a duplicate of an existing image, the input image is either
        ///    discarded or used as a replacement for that existing image, depending upon the
        ///    outcome of a decision as in (3); otherwise, no action is taken.
        ///
        /// The process may be interrupted at any point by requesting an interruption on the
        /// engine's thread. If \p input_image_paths contains paths that are descendants of
        /// \p collection_path, they are treated as part of the collection and not as inputs.
        ///

        Q_INVOKABLE
        void merge(const QStringList& input_image_paths, const QString& collection_path) const;

    Q_SIGNALS:

        void input_count_changed(int file_count, int folder_count) const;
        void phase_changed(phase new_phase) const;
        void progress_changed(int percent_complete) const;

    private:

        ///
        /// Compares images as specified by \p pair_strategy, emitting the progress_changed() signal
        /// to indicate how close to completion this process is. A single merge operation may use a
        /// sequence of different \p pairer strategies; because of this, a call to compare_images()
        /// need not take the emitted progress from 0 to 100. \p start_count specifies how many
        /// images have already been compared before this particular call to compare_images() was
        /// made; \p total_count specifies how many images need to be compared before that phase of
        /// the merge operation is considered complete. This operation may be interrupted by
        /// requesting an interruption on the engine's thread.
        ///

        int compare_images(pairer& pair_strategy, int start_count, int total_count) const;

        ///
        /// Constructs an \ref image_info object for each filesystem path in \p paths, emitting the
        /// progress_changed() signal to indicate how close to completion this process is. A single
        /// merge operation may hash more than one group of image paths; because of this, a call to
        /// hash_images() need not take the emitted percentage progress from 0 to 100.
        /// \p start_count specifies how many images have already been hashed before this particular
        /// call to hash_images() was made; \p total_count specifies how many images need to be
        /// hashed before that phase of the merge operation is considered complete. This operation
        /// may be interrupted by requesting an interruption on the engine's thread.
        ///

        image_set hash_images(
            const QStringList &paths, int start_count, int total_count) const;

        ///
        /// If \p base_path is the filesystem path to a directory, recursively scans the descendants
        /// of that directory and appends to \p image_paths the paths of all supported image files
        /// therein; if \p base_path is the path to a file, that file is itself appended to
        /// \p image_paths if it is supported. \p folder_count is set to the number of directories
        /// that were scanned during this operation. As this scan is performed, the
        /// input_count_changed() signal is emitted to indicate how many supported image files have
        /// been found and how many directories have been scanned. This operation may be interrupted
        /// by requesting an interruption on the engine's thread.
        ///

        void scan_for_images(
            const QString &base_path, QStringList &image_paths, int &folder_count) const;

        ///
        /// Emits the phase_changed() signal to indicate that a merge operation being performed by
        /// the engine has entered a new phase, and also emits a progress_changed() signal to reset
        /// the current progress to zero accordingly.
        ///

        void signal_phase_change(phase new_phase) const;

        ///
        /// Emits the input_count_changed() signal to indicate that progress has been made on a scan
        /// operation. Since the input count changes very rapidly during a scan, an emission is not
        /// made for every increment in \p file_count and \p folder_count; rather, a minimum
        /// interval is enforced between emissions. However, once the final file and folder counts
        /// have been determined, it is important that these are accurately communicated; when this
        /// happens, \p type should be set to ksr::dense_update_type::final to force an emission.
        ///

        void signal_scan_progress(
            int file_count, int folder_count,
            ksr::dense_update_type type = ksr::dense_update_type::transient) const;

        const ksr::dense_updater<std::chrono::milliseconds> m_dense_updater;
        const ksr::progress_updater m_progress_updater;
    };

    bool thread_interrupted();
}

#endif
