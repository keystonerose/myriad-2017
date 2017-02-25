#ifndef MYRIAD_IMAGE_ATTR_HPP
#define MYRIAD_IMAGE_ATTR_HPP

#include <cstdint>

class QString;

namespace myriad {

    ///
    /// Identifies specific image formats that are associated with special appraisal logic. While
    /// Myriad supports all formats supported by \c QImage, most of these receive no special
    /// treatment and are therefore grouped under the \c other enumerator.
    ///
    
    enum class image_format { other, bmp, gif, jpeg, png };

    ///
    /// Aggregates the attributes used in the comparison of images. The \c phash member stores the
    /// perceptual hashes used to determine how similar images are; the other members are associated
    /// with logic used to determine which images are preferable when they are considered to be
    /// different.
    ///
    
    struct image_attr {
        
        int width;
        int height;

        std::uint16_t checksum;
        std::uint64_t file_size;
        image_format format;
        std::uint64_t phash;
    };

    ///
    /// Populates an \ref image_attr object describing the image located at the filesystem path
    /// \p image_path. Since these attributes include the perceptual hash and checksum of the image,
    /// this is an expensive operation.
    /// \throws file_io_error if data could not be read from \p image_path.
    ///

    image_attr read_attr(const QString& image_path);
}

#endif

