#include "image_info.hpp"
#include "exception.hpp"
#include "hash.hpp"

#include "pHash.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <QMimeDatabase>
#include <QTemporaryFile>

#include <unordered_map>

namespace myriad {

    namespace {

        ///
        /// Generates a checksum for the contents of the file at the filesystem path \p path (which
        /// may be zero if the file was able to be opened but no data were able to read from it).
        /// \throws file_io_error if the file could not be opened.
        ///

        uint16_t checksum(const QString& path) {

            QFile file{path};
            if (!file.open(QIODevice::ReadOnly)) {
                throw file_io_error{path};
            }

            const auto raw_data  = file.readAll();
            return qChecksum(raw_data, raw_data.length());
        }

        ///
        /// Determines an \ref image_format code identifying the format of the image file at the
        /// filesystem path \p path. If the file does not have a recognised image (or non-image)
        /// MIME type, \c image_format::other is returned (and it is the responsibility of calling
        /// code to detect this error through other means).
        ///

        image_format format(const QString& path) {

            static const std::unordered_map<QString, image_format> formats_by_mime_name{
                {QStringLiteral("image/bmp"),  image_format::bmp},
                {QStringLiteral("image/gif"),  image_format::gif},
                {QStringLiteral("image/jpeg"), image_format::jpeg},
                {QStringLiteral("image/png"),  image_format::png}
            };

            const QMimeDatabase mime_db;
            const auto mime_name = mime_db.mimeTypeForFile(path).name();

            const auto iter = formats_by_mime_name.find(mime_name);
            return (iter == std::cend(formats_by_mime_name)) ? image_format::other : iter->second;
        }
    }

    image_info::image_info(const QString& path)
        : m_file_info{path} {

        const QImage image{path};
        if (image.isNull()) {
            throw file_io_error{path};
        }

        m_width = image.width();
        m_height = image.height();

        m_checksum = myriad::checksum(path);
        m_format = myriad::format(path);

        // While ulong64 and std::uint64_t are specified to be compatible types, there is
        // potentially a conversion between the two that prevents a reference to result.phash from
        // being passed directly to ph_dct_imagehash().

        ulong64 phash = 0;

        // The pHash library only supports JPEG and BMP files, so if the image is not already in one
        // of those formats, a temporary bitmap file is created and used to calculate the hash
        // value.

        if (m_format == image_format::jpeg || m_format == image_format::bmp) {
            ph_dct_imagehash(path.toLatin1(), phash);
        } else {

            QTemporaryFile temp_bmp{QDir::tempPath() + "/myriad_XXXXXX.bmp"};
            const auto temp_path = QFileInfo{temp_bmp}.filePath();
            if (!temp_bmp.open()) {
                throw file_io_error{temp_path};
            }

            image.save(&temp_bmp, "BMP");
            temp_bmp.close();
            ph_dct_imagehash(temp_path.toLatin1(), phash);
        }

        m_phash = phash;
    }

    bool operator==(const image_info& lhs, const image_info& rhs) {
        return lhs.m_file_info == rhs.m_file_info;
    }

    bool operator!=(const image_info& lhs, const image_info& rhs) {
        return !(lhs == rhs);
    }
}
