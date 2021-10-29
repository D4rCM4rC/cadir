include(ExternalProject)

cmake_minimum_required(VERSION 3.13)
project(cadir3)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

#########################
## LIBARCHIVE ## BEGIN ##

set(LIB_ARCHIVE_DISABLED_MODULES --without-bz2lib --without-iconv --without-libb2 --without-lz4 --without-zstd --without-lzma --without-cng --without-xml2 --without-expat)
set(LIB_ARCHIVE_EXT_LIBS archive z)

ExternalProject_Add(
        lib_archive
        URL https://github.com/libarchive/libarchive/releases/download/v3.5.2/libarchive-3.5.2.tar.gz
        URL_MD5
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --srcdir=<SOURCE_DIR> ${LIB_ARCHIVE_DISABLED_MODULES} --prefix=<INSTALL_DIR>
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
)
ExternalProject_Get_Property(lib_archive INSTALL_DIR)

set(LIB_ARCHIVE_INSTALL_DIR ${INSTALL_DIR})
file(MAKE_DIRECTORY ${LIB_ARCHIVE_INSTALL_DIR}/include)

add_library(archive STATIC IMPORTED GLOBAL)
set_target_properties(archive PROPERTIES IMPORTED_LOCATION ${LIB_ARCHIVE_INSTALL_DIR}/lib/libarchive.a)
set_target_properties(archive PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LIB_ARCHIVE_INSTALL_DIR}/include)

add_dependencies(archive lib_archive)

## LIBARCHIVE ## END ##
#######################


#####################
# OPENSSL ## BEGIN ##
find_package(OpenSSL REQUIRED)
if ( OPENSSL_FOUND )
    message(STATUS "OpenSSL Found: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
endif()

include_directories(${OPENSSL_INCLUDE_DIR})

# OPENSSL ## END ##
#####################


link_libraries(${OPENSSL_LIBRARIES} ${LIB_ARCHIVE_EXT_LIBS})

add_executable(cadir3 main.cpp Exceptions/FileHandlingException.h Exceptions/SetupCommandException.h Exceptions/CreateCacheDirectoryException.h Exceptions/CopyToCacheFailedException.h Exceptions/CleaningFailedException.h Exceptions/CopyFromCacheException.h Exceptions/LinkFromCacheException.h compress.hpp fileSystem.hpp exitCodeEnum.hpp)
set(CMAKE_VERBOSE_MAKEFILE ON)