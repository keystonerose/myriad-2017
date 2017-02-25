#ifndef MYRIAD_EXCEPTION_HPP
#define MYRIAD_EXCEPTION_HPP

#include <stdexcept>
#include <string>

class QString;

namespace myriad {
    
    ///
    /// Thrown when an error occurs reading data from or writing data to a file on disk. The
    /// <tt>what()</tt> member function returns the filesystem path to the file that could not be
    /// read from or written to.
    ///

    struct file_io_error : std::runtime_error {
        explicit file_io_error(const std::string& path);
        explicit file_io_error(const QString& path);
    };
}

#endif
