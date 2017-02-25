#include "engine.hpp"

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

using namespace std::chrono_literals;

namespace myriad {
    
    namespace {

        const QList<QByteArray> &supported_mime_types();
        
        ///
        /// Determines whether the file at the filesystem path \p path is an image file able to be
        /// processed by Myriad. May only be called after the \c QApplication instance has been
        /// created.
        ///
        
        bool file_supported(const QString &path) {
            
            const QMimeDatabase mime_db;
            const auto mime_name = mime_db.mimeTypeForFile(path).name();

            const auto &supported = supported_mime_types();
            return supported.contains(mime_name.toLatin1());
        }
        
        int int_percentage(const int num, const int denom) {
            const auto result = std::lround(100.0f * static_cast<float>(num) / static_cast<float>(denom));
            return gsl::narrow_cast<int>(result);
        }

        bool interrupted() {
            return QThread::currentThread()->isInterruptionRequested();
        }

        ///
        /// Gets a list of all image MIME types that Myriad is able to process. May only be called
        /// after the \c QApplication instance has been created.
        ///
        
        const QList<QByteArray> &supported_mime_types() {
            static auto result = QImageReader::supportedMimeTypes();
            return result;
        }
    }
    
    std::vector<image_info> engine::hash_images(
        const QStringList &paths, const int start_count, const int total_count) const {
        
        std::vector<image_info> result;
        auto last_percent_complete = int_percentage(start_count, total_count);
            
        const auto begin = std::cbegin(paths);
        const auto end = std::cend(paths);
        for (auto iter = begin; iter != end && !interrupted();) {

            result.emplace_back(*iter++);

            const auto hashed_count = start_count + std::distance(begin, iter);
            const auto percent_complete = int_percentage(hashed_count, total_count);
            
            if (percent_complete > last_percent_complete) {
                Q_EMIT progress_changed(percent_complete);
                last_percent_complete = percent_complete;
            }
        }
        
        return result;
    }

    void engine::merge(QStringList input_image_paths, const QString &collection_path) const {
        
        QStringList collection_image_paths;
        int folder_count = 0;

        signal_phase_change(phase::scan);

        signal_scan_progress(collection_image_paths.size(), folder_count);
        scan_for_images(collection_path, collection_image_paths, folder_count);
        signal_scan_progress(collection_image_paths.size(), folder_count, ksr::dense_update_type::final);
        
        signal_phase_change(phase::hash);

        const auto total_count = input_image_paths.size() + collection_image_paths.size();
        auto inputs = hash_images(input_image_paths, 0, total_count);
        auto collection = hash_images(collection_image_paths, inputs.size(), total_count);
        
        // The actual order of objects in collection is unimportant, but they must be able to be
        // ordered quickly.
        
        const auto less = [](const image_info& lhs, const image_info& rhs) {
            return lhs.checksum() < rhs.checksum();
        };
        
        std::sort(std::begin(collection), std::end(collection), less);

        // Removing items from inputs that already exist in the collection is conceptually a
        // std::set_difference()-like operation; however, input_image_paths is unsorted (and should
        // remain so), so we can't apply this algorithm to it directly, and there's no need to
        // preserve the objects that lie in the intersection.

        const auto in_collection = [&collection, less](const image_info &item) {
            return std::binary_search(std::cbegin(collection), std::cend(collection), item, less);
        };

        inputs.erase(
            std::remove_if(std::begin(inputs), std::end(inputs), in_collection),
            std::end(inputs));
    }

    void engine::scan_for_images(
        const QString &base_path, QStringList &image_paths, int &folder_count) const {
            
        const QFileInfo info{base_path};
        if (!info.exists()) {
            return;
        }
        
        if (info.isFile()) {

            if (file_supported(base_path)) {
                image_paths.push_back(base_path);
                signal_scan_progress(image_paths.size(), folder_count);
            }
        }
        else if (info.isDir()) {
            
            QDir dir{base_path};
            dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
            
            const auto items = dir.entryInfoList();
            ++folder_count;
            signal_scan_progress(image_paths.size(), folder_count);

            const auto end = std::cend(items);
            for (auto iter = std::cbegin(items); iter != end && !interrupted(); ++iter) {
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

        static auto emitter = ksr::make_dense_updater(
            20ms, [this](const int file_count, const int folder_count) {
                Q_EMIT input_count_changed(file_count, folder_count);
            });

        emitter.try_update(type, file_count, folder_count);
    }
}

