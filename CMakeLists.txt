cmake_minimum_required(VERSION 3.6)
project(myriad)

# The explicit compiler setup here isn't ideal for building on other systems, but since Myriad
# targets C++17 (via Clang 5 and libstdc++ 7 on my machine, neither of which has yet seen an
# official release), the setup here needs to be a little unusual for now. Once C++17 toolchains are
# more widely available, the need for a specialised setup here will be obviated.

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

set(CLANG_FLAGS "--gcc-toolchain=/opt/gcc7 -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CLANG_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z ${CLANG_FLAGS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(3RDPARTY_DIR "3rdparty")
set(INSTALL_DIR "install")
set(PHASH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${3RDPARTY_DIR}/libphash")
set(PHASH_SRC_DIR "${PHASH_DIR}/src/libphash")
set(PHASH_LIBRARY "${PHASH_DIR}/lib/libpHash.a")

find_program(CLANG_TIDY_PATH NAMES "clang-tidy")
if(NOT CLANG_TIDY_PATH)
    message(STATUS "Could not find clang-tidy")
else()
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_PATH}")
endif()

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

find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)

# To enforce C++11 support, Qt appends -std=gnu++11 to CXX_FLAGS if CMAKE_CXX_VERSION hasn't been
# set appropriately for it. However, CMAKE_CXX_VERSION doesn't yet support C++17, so we have to
# specify this via the -std=c++1z compiler flags, which Qt will happily override. To prevent it from
# doing this, we tell Qt5::Core to forget about requiring specific C++11 compiler features (which
# we're ensuring for it in this case).

find_package(Qt5 REQUIRED COMPONENTS Widgets Test)
set_property(TARGET Qt5::Core PROPERTY INTERFACE_COMPILE_FEATURES "")

include_directories(src)
include_directories(SYSTEM ${3RDPARTY_DIR}/GSL/include)
include_directories(SYSTEM ${PHASH_DIR}/include)

add_subdirectory(src)
add_subdirectory(src/test_ksr)
add_executable(myriad ${MYRIAD_SRCS})
add_dependencies(myriad libphash)

# The integrated CXX_CLANG_TIDY target property provided by CMake 3.6 and greater causes checks to
# be run every time the project is built, dramatically increasing the build time. It's much less
# frustrating to be able to enable these checks selectively.

set(MYRIAD_ENABLE_LINT OFF CACHE BOOL "Enable static analysis checks during build")
if(CLANG_TIDY_PATH AND MYRIAD_ENABLE_LINT)
    set_target_properties(myriad PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_PATH}")
endif()

target_link_libraries(myriad Qt5::Widgets ${PHASH_LIBRARY} ${PNG_LIBRARY} ${JPEG_LIBRARIES})
install(TARGETS myriad RUNTIME DESTINATION bin)
