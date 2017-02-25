#include "exception.hpp" 
#include <QString>
 
 namespace myriad {
 
    file_io_error::file_io_error(const std::string& path)
        : runtime_error{path} {}

    file_io_error::file_io_error(const QString& path)
        : runtime_error{path.toLatin1()} {}
 }
 
