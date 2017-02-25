#include "image_attr.hpp"
#include "exception.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "pHash.h"
#pragma GCC diagnostic pop

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QMimeDatabase>
#include <QString>
#include <QTemporaryFile>

#include "gsl/gsl"
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
            
            auto close_act = gsl::finally([&file] { file.close(); });

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
            
            static const QHash<QString, image_format> formats_by_mime_name{
                {QStringLiteral("image/bmp"),  image_format::bmp},
                {QStringLiteral("image/gif"),  image_format::gif},
                {QStringLiteral("image/jpeg"), image_format::jpeg},
                {QStringLiteral("image/png"),  image_format::png}
            };

            const QMimeDatabase mime_db;
            const auto mime_name = mime_db.mimeTypeForFile(path).name();

            const auto iter = formats_by_mime_name.find(mime_name);
            return (iter == std::cend(formats_by_mime_name)) ? image_format::other : *iter;
        }
    }

    image_attr read_attr(const QString& image_path) {
        
        image_attr result;
        
        const QFileInfo info{image_path};
        const QImage image{image_path};
        if (image.isNull()) {
            throw file_io_error{image_path};
        }
        
        result.width = image.width();
        result.height = image.height();
        
        result.checksum = checksum(image_path);
        result.file_size = info.size();
        result.format = format(image_path);
        
        // While ulong64 and std::uint64_t are specified to be compatible types, there is
        // potentially a conversion between the two that prevents a reference to result.phash from
        // being passed directly to ph_dct_imagehash().
        
        ulong64 phash = 0;
        
        // The pHash library only supports JPEG and BMP files, so if the image is not already in one
        // of those formats, a temporary bitmap file is created and used to calculate the hash
        // value.

        if (result.format == image_format::jpeg || result.format == image_format::bmp) {
            ph_dct_imagehash(image_path.toLatin1(), phash);
        }
        else {

            QTemporaryFile temp_bmp{QDir::tempPath() + "/myriad_XXXXXX.bmp"};
            const auto temp_path = QFileInfo{temp_bmp}.filePath();
            if (!temp_bmp.open()) {
                throw file_io_error{temp_path};
            }
            
            image.save(&temp_bmp, "BMP");
            temp_bmp.close();
            ph_dct_imagehash(temp_path.toLatin1(), phash);
        }
        
        result.phash = phash;
        return result;
    }
}
