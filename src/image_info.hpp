#ifndef MYRIAD_IMAGE_ATTR_HPP
#define MYRIAD_IMAGE_ATTR_HPP

#include <QFileInfo>
#include <QString>

#include <cstdint>

namespace myriad {

    ///
    /// Identifies specific image formats that are associated with special appraisal logic. While
    /// Myriad supports all formats supported by \c QImage, most of these receive no special
    /// treatment and are therefore grouped under the \c other enumerator.
    ///

    enum class image_format { other, bmp, gif, jpeg, png };

    ///
    /// Reads and stores information used to identify and compare the images processed by Myriad. In
    /// particular, the phash() member function returns a perceptual hashe that may used to
    /// determine how similar an image is to another; the other properties are associated with logic
    /// used to determine which images are preferable when they are considered to be different. All
    /// of the significant work done by an \ref image_info object is performed upon construction;
    /// after that point, attributes may be queried from the object at negligible performance cost.
    ///

    class image_info {
    public:

        ///
        /// Determines whether two \ref image_info objects describe files in the same location on
        /// disk. Note that this is a stronger criterion than \p lhs and \p rhs being bytewise
        /// equivalent (the weaker condition may be assessed by comparison of their checksums).
        ///
        friend bool operator==(const image_info& lhs, const image_info& rhs);
        friend bool operator!=(const image_info& lhs, const image_info& rhs);

        ///
        /// Fetches information about the image file at the filesystem path \p path and constructs
        /// an \ref image_info object to store that information. Since the stored image attributes
        /// include the perceptual hash of the image, this is an expensive operation.
        /// \throws file_io_error if image data could not be read from \p path.
        ///

        explicit image_info(const QString& path);

        std::uint16_t checksum() const {
            return m_checksum;
        }

        std::uint64_t file_size() const {
            return m_file_info.size();
        }

        image_format format() const {
            return m_format;
        }

        int height() const {
            return m_height;
        }

        QString path() const {
            return m_file_info.absoluteFilePath();
        }

        std::uint64_t phash() const {
            return m_phash;
        }

        int width() const {
            return m_width;
        }

    private:

        int m_width = 0;
        int m_height = 0;

        image_format m_format = image_format::other;
        std::uint16_t m_checksum = 0;
        std::uint64_t m_phash = 0;

        QFileInfo m_file_info;
    };
}

#endif

