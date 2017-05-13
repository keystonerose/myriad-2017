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

using namespace std::literals::chrono_literals;

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

        ///
        /// Gets a list of all image MIME types that Myriad is able to process. May only be called
        /// after the \c QApplication instance has been created.
        ///

        const QList<QByteArray>& supported_mime_types() {
            static auto result = QImageReader::supportedMimeTypes();
            return result;
        }
    }

    phase_data<phase::scan>::phase_data(const engine& self)
      : signaller{20ms, [&self](const int file_count, const int folder_count) {
            Q_EMIT self.input_count_changed(file_count, folder_count);
        }} {}

    phase_data<phase::hash>::phase_data(const engine& self)
      : signaller{[&self](const auto num, const auto denom) {
            Q_EMIT self.progress_changed(ksr::int_percentage(num, denom));
        }} {}

    phase_data<phase::compare>::phase_data(const engine& self)
      : signaller{[&self](const auto num, const auto denom) {
            Q_EMIT self.progress_changed(ksr::int_percentage(num, denom));
        }} {}

    engine::engine()
      : m_input_signaller{20ms, [this](const int file_count, const int folder_count) {
            Q_EMIT input_count_changed(file_count, folder_count);
        }} {}

    template <typename pairer_t>
    void engine::compare_images(
        pairer_t& pairer, ksr::int_percentage_filter<int>& progress_signaller) const {

        // TODO Currently, total_count is not updated when images are removed and the number that
        // is eventually processed ultimately changes; we need a mechanism for keeping that value
        // up-to-date.

        auto count = progress_signaller.count();
        pairer.pair([&](const image_info&, const image_info&) {
            const auto total_count = progress_signaller.total_count();
            progress_signaller.update(++count, total_count);
        });
    }

    image_set engine::hash_images(
        const QStringList& paths, ksr::int_percentage_filter& progress_signaller) const {

        auto result = image_set{};

        const auto start_count = progress_signaller.count();
        const auto total_count = progress_signaller.total_count();

        const auto begin = std::cbegin(paths);
        const auto end = std::cend(paths);
        for (auto iter = begin; iter != end && !thread_interrupted();) {
            result.emplace(*iter++);
            progress_signaller.update(start_count + std::distance(begin, iter), total_count);
        }

        return result;
    }

    void engine::merge(const QStringList& input_image_paths, const QString& collection_path) const {

        auto collection_image_paths = QStringList{};
        auto folder_count = 0;

        signal_phase_change(phase::scan);
        m_input_signaller.sync(0, 0);

        scan_for_images(collection_path, collection_image_paths, folder_count);
        m_input_signaller.sync(collection_image_paths.size(), folder_count);

        auto progress_signaller = ksr::int_percentage_filter<int>{
            [](const auto num, const auto denom) {
                Q_EMIT progress_changed(ksr::int_percentage(num, denom));
            }};

        signal_phase_change(phase::hash);
        progress_signaller.sync(0, image_count);

        const auto image_count = input_image_paths.size() + collection_image_paths.size();
        auto inputs = hash_images(input_image_paths, progress_signaller);
        auto collection = hash_images(collection_image_paths, progress_signaller);

        ksr::erase_if(inputs, [&collection](const image_info& item) {
            return collection.count(item) > 0;
        });

        auto deduplicator = deduplicate_pairer{collection};
        auto merger = merge_pairer{inputs, collection};

        signal_phase_change(phase::compare);
        progress_signaller.sync(0, deduplicator.count() + merger.count());

        compare_images(deduplicator, progress_signaller);
        compare_images(merger, progress_signaller);
    }

    void engine::scan_for_images(
        const QString &base_path, QStringList &image_paths, int &folder_count) const {

        const auto info = QFileInfo{base_path};
        if (!info.exists()) {
            return;
        }
1
        if (info.isFile()) {

            if (file_supported(base_path)) {
                image_paths.push_back(base_path);
                m_input_signaller.update(image_paths.size(), folder_count);
            }

        } else if (info.isDir()) {

            auto dir = QDir{base_path};
            dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

            const auto items = dir.entryInfoList();
            ++folder_count;
            m_input_signaller.update(image_paths.size(), folder_count);

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

    bool thread_interrupted() {
        return QThread::currentThread()->isInterruptionRequested();
    }
}

