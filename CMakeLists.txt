include(ExternalProject)

cmake_minimum_required(VERSION 3.13)
project(cadir3)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

##########
try_compile(HAS_FILESYSTEM "${CMAKE_BINARY_DIR}/temp" "${CMAKE_SOURCE_DIR}/compiletest/has_filesystem.cpp"
        CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
        LINK_LIBRARIES stdc++fs)


configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#########################
## LIBARCHIVE ## BEGIN ##

set(LIB_ARCHIVE_DISABLED_MODULES --disable-acl --without-bz2lib --without-iconv --without-libb2 --without-lz4 --without-zstd --without-lzma --without-cng --without-xml2 --without-expat)
set(LIB_ARCHIVE_EXT_LIBS archive z)

ExternalProject_Add(
        lib_archive
        URL https://github.com/libarchive/libarchive/releases/download/v3.5.2/libarchive-3.5.2.tar.gz
        URL_MD5 73b093ce0ac09bb73b1ab58f02e95db7
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --srcdir=<SOURCE_DIR> ${LIB_ARCHIVE_DISABLED_MODULES} --prefix=<INSTALL_DIR>
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
        BUILD_BYPRODUCTS <INSTALL_DIR>/lib/libarchive.a
)
ExternalProject_Get_Property(lib_archive INSTALL_DIR)

set(LIB_ARCHIVE_INSTALL_DIR ${INSTALL_DIR})
file(MAKE_DIRECTORY ${LIB_ARCHIVE_INSTALL_DIR}/include)

add_library(archive STATIC IMPORTED GLOBAL compiletest/has_filesystem.cpp)
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

add_executable(cadir3 main.cpp config.h.in)
set(CMAKE_VERBOSE_MAKEFILE ON)
