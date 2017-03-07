#include "engine.hpp"

#include "ksr/algorithm.hpp"

#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QList>
#include <QMimeDatabase>
#include <QString>
#include <QThread>
#include <QTimer>

#include "gsl/gsl"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iterator>
#include <numeric>

using namespace std::chrono_literals;

namespace myriad {

    namespace {

        const QList<QByteArray>& supported_mime_types();

        ///
        /// Determines whether the file at the filesystem path \p path is an image file able to be
        /// processed by Myriad. May only be called after the \c QApplication instance has been
        /// created.
        ///

        bool file_supported(const QString& path) {

            const QMimeDatabase mime_db;
            const auto mime_name = mime_db.mimeTypeForFile(path).name();

            const auto& supported = supported_mime_types();
            return supported.contains(mime_name.toLatin1());
        }

        int int_percentage(const int num, const int denom) {
            const auto result = std::lround(100.0f * static_cast<float>(num) / static_cast<float>(denom));
            return gsl::narrow_cast<int>(result);
        }

        ///
        /// Gets a list of all image MIME types that Myriad is able to process. May only be called
        /// after the \c QApplication instance has been created.
        ///

        const QList<QByteArray>& supported_mime_types() {
            static auto result = QImageReader::supportedMimeTypes();
            return result;
        }
    }

    engine::engine()
      : m_updater{20ms} {
    }

    int engine::compare_images(
        pairer& pair_strategy, const int start_count, const int total_count) const {

        auto count = start_count;
        auto last_percent_complete = int_percentage(count, total_count);

        // TODO Currently, total_count is not updated when images are removed and the number that
        // is eventually processed ultimately changes; we need a mechanism for keeping that value
        // up-to-date.

        pair_strategy.pair(
            [this, &count, &last_percent_complete, total_count]
            (const image_info&, const image_info&) {

                const auto percent_complete = int_percentage(count, total_count);
                if (percent_complete > last_percent_complete) {
                    Q_EMIT progress_changed(percent_complete);
                    last_percent_complete = percent_complete;
                }

                ++count;
                return discard_choice::none;
            });

        return count;
    }

    image_set engine::hash_images(
        const QStringList& paths, const int start_count, const int total_count) const {

        auto result = image_set{};
        auto last_percent_complete = int_percentage(start_count, total_count);

        const auto begin = std::cbegin(paths);
        const auto end = std::cend(paths);
        for (auto iter = begin; iter != end && !thread_interrupted();) {

            result.emplace(*iter++);

            const auto hashed_count = start_count + std::distance(begin, iter);
            const auto percent_complete = int_percentage(hashed_count, total_count);

            if (percent_complete > last_percent_complete) {
                Q_EMIT progress_changed(percent_complete);
                last_percent_complete = percent_complete;
            }
        }

        return result;
    }

    void engine::merge(const QStringList& input_image_paths, const QString& collection_path) const {

        auto collection_image_paths = QStringList{};
        auto folder_count = 0;

        signal_phase_change(phase::scan);
        signal_scan_progress(0, 0);

        scan_for_images(collection_path, collection_image_paths, folder_count);
        signal_scan_progress(collection_image_paths.size(), folder_count, ksr::dense_update_type::final);

        signal_phase_change(phase::hash);

        const auto image_count = input_image_paths.size() + collection_image_paths.size();
        auto inputs = hash_images(input_image_paths, 0, image_count);
        auto collection = hash_images(collection_image_paths, inputs.size(), image_count);

        ksr::erase_if(inputs, [&collection](const image_info& item) {
            return collection.count(item) > 0;
        });

        signal_phase_change(phase::compare);

        auto deduplicator = deduplicate_pairer{collection};
        auto merger = merge_pairer{inputs, collection};

        auto count = 0;
        const auto comp_count = deduplicator.count() + merger.count();
        count = compare_images(deduplicator, count, comp_count);
        compare_images(merger, count, comp_count);
    }

    void engine::scan_for_images(
        const QString &base_path, QStringList &image_paths, int &folder_count) const {

        const auto info = QFileInfo{base_path};
        if (!info.exists()) {
            return;
        }

        if (info.isFile()) {

            if (file_supported(base_path)) {
                image_paths.push_back(base_path);
                signal_scan_progress(image_paths.size(), folder_count);
            }

        } else if (info.isDir()) {

            auto dir = QDir{base_path};
            dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

            const auto items = dir.entryInfoList();
            ++folder_count;
            signal_scan_progress(image_paths.size(), folder_count);

            const auto end = std::cend(items);
            for (auto iter = std::cbegin(items); iter != end && !thread_interrupted(); ++iter) {
                scan_for_images(iter->absoluteFilePath(), image_paths, folder_count);
            }
        }
    }

    void engine::signal_phase_change(const phase new_phase) const {
        Q_EMIT phase_changed(new_phase);
        Q_EMIT progress_changed(0);
    }

    void engine::signal_scan_progress(
        const int file_count, const int folder_count, const ksr::dense_update_type type) const {

        m_updater.try_update(type, [this, file_count, folder_count] {
            Q_EMIT input_count_changed(file_count, folder_count);
        });
    }

    bool thread_interrupted() {
        return QThread::currentThread()->isInterruptionRequested();
    }
}

