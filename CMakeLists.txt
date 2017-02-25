cmake_minimum_required(VERSION 2.8.11)
project(myriad)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fexceptions")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(3RDPARTY_DIR "3rdparty")
set(INSTALL_DIR "install")
set(PHASH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${3RDPARTY_DIR}/libphash")
set(PHASH_SRC_DIR "${PHASH_DIR}/src/libphash")
set(PHASH_LIBRARY "${PHASH_DIR}/lib/libpHash.a")

include(ExternalProject)

# libphash does not provide an official git repository; Debian has one for its own packaged version,
# but it doesn't support out-of-tree builds well and tracks its generated configure script, causing
# a submodule tracking that repository to immediately become dirty when built.

ExternalProject_Add(libphash
    PREFIX ${PHASH_DIR}
    SOURCE_DIR ${PHASH_SRC_DIR}    
    URL http://www.phash.org/releases/pHash-0.9.6.tar.gz
    CONFIGURE_COMMAND ${PHASH_SRC_DIR}/configure --disable-video-hash --disable-audio-hash --disable-pthread --prefix=${PHASH_DIR}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)

# libjpeg and libpng are not used directly, but required by libphash.

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)

include_directories(${3RDPARTY_DIR}/GSL/include)
include_directories(${PHASH_DIR}/include)

add_subdirectory(src)
add_executable(myriad ${MYRIAD_SRCS})
add_dependencies(myriad libphash)

target_link_libraries(myriad Qt5::Widgets ${PHASH_LIBRARY} ${PNG_LIBRARY} ${JPEG_LIBRARIES})
install(TARGETS myriad RUNTIME DESTINATION bin)
