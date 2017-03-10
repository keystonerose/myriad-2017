#include "hash.hpp"
#include "image_info.hpp"

#include <QString>

using myriad::image_info;

namespace std {

    // Unfortunately, qHash() only provides an int, rather than std::size_t, hash, so on most modern
    // platforms we're only using half of the bits we have available here. An alternative would be
    // to convert value into a std::string and hash that in the standard way, but performing a
    // (potential) allocation inside a hash function isn't great, and the hash provided by qHash()
    // is good enough for our purposes.

    std::size_t hash<QString>::operator()(const QString& value) const noexcept {
        return qHash(value);
    }

    std::size_t hash<image_info>::operator()(const image_info& value) const noexcept {
        return hash<QString>{}(value.path());
    }
}
