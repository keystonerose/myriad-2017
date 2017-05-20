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
#include <variant>

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

    template <phase new_phase>
    auto engine::change_phase() const -> phase_data<new_phase>& {

        auto& data = m_data.emplace<phase_data<new_phase>>();
        Q_EMIT phase_changed(new_phase);
        Q_EMIT progress_changed(0);
        return data;
    }

    template <phase current_phase>
    auto engine::current_data() const -> phase_data<current_phase>& {

        using data_t = phase_data<current_phase>;
        assert(std::holds_alternative<data_t>(m_data));
        return std::get<data_t>(m_data);
    }

    template <typename pairer_t>
    void engine::compare_images(pairer_t& pairer) const {

        // TODO Currently, total_count is not updated when images are removed and the number that
        // is eventually processed ultimately changes; we need a mechanism for keeping that value
        // up-to-date.

        auto& signaller = current_data<phase::compare>().signaller;
        auto count = signaller.count();

        pairer.pair([&](const image_info&, const image_info&) {
            signaller.update(++count, signaller.total());
        });
    }

    auto engine::hash_images(const QStringList& paths) const -> image_set {

        auto result = image_set{};

        auto& signaller = current_data<phase::hash>().signaller;
        const auto start_count = signaller.count();
        const auto total_count = signaller.total();

        const auto begin = std::cbegin(paths);
        const auto end = std::cend(paths);
        for (auto iter = begin; iter != end && !thread_interrupted();) {
            result.emplace(*iter++);
            signaller.update(start_count + std::distance(begin, iter), total_count);
        }

        return result;
    }

    void engine::merge(const QStringList& input_image_paths, const QString& collection_path) const {

        auto collection_image_paths = QStringList{};
        auto folder_count = 0;

        auto& scan_signaller = change_phase<phase::scan>().signaller;
        scan_signaller.sync(0, 0);

        scan_for_images(collection_path, collection_image_paths, folder_count);
        scan_signaller.sync(collection_image_paths.size(), folder_count);

        auto& hash_signaller = change_phase<phase::hash>().signaller;
        hash_signaller.sync(0, image_count);

        const auto image_count = input_image_paths.size() + collection_image_paths.size();
        auto inputs = hash_images(input_image_paths, progress_signaller);
        auto collection = hash_images(collection_image_paths, progress_signaller);

        ksr::erase_if(inputs, [&collection](const image_info& item) {
            return collection.count(item) > 0;
        });

        auto deduplicator = deduplicate_pairer{collection};
        auto merger = merge_pairer{inputs, collection};

        auto& compare_signaller = change_phase<phase::compare>().signaller;
        compare_signaller.sync(0, deduplicator.count() + merger.count());

        compare_images(deduplicator);
        compare_images(merger);
    }

    void engine::scan_for_images(
        const QString &base_path, QStringList &image_paths, int &folder_count) const {

        const auto info = QFileInfo{base_path};
        if (!info.exists()) {
            return;
        }

        auto& signaller = current_data<phase::scan>().signaller;
        if (info.isFile()) {

            if (file_supported(base_path)) {
                image_paths.push_back(base_path);
                signaller.update(image_paths.size(), folder_count);
            }

        } else if (info.isDir()) {

            auto dir = QDir{base_path};
            dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

            const auto items = dir.entryInfoList();
            ++folder_count;
            signaller.update(image_paths.size(), folder_count);

            const auto end = std::cend(items);
            for (auto iter = std::cbegin(items); iter != end && !thread_interrupted(); ++iter) {
                scan_for_images(iter->absoluteFilePath(), image_paths, folder_count);
            }
        }
    }

    auto thread_interrupted() -> bool {
        return QThread::currentThread()->isInterruptionRequested();
    }
}

