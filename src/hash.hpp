#ifndef MYRIAD_HASH_HPP
#define MYRIAD_HASH_HPP

#include <cstddef>
#include <functional>

class QString;
namespace myriad {
    class image_info;
}

namespace std {

    template <>
    struct hash<QString> {
        std::size_t operator()(const QString& value) const noexcept;
    };

    template <>
    struct hash<myriad::image_info> {
        std::size_t operator()(const myriad::image_info& value) const noexcept;
    };
}

#endif
