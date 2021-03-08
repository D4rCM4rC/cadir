cmake_minimum_required(VERSION 3.17)
project(cadir3)

set(CMAKE_CXX_STANDARD 17)
set( OPENSSL_ROOT_DIR "/usr/local/opt/openssl@1.1")

find_package(OpenSSL REQUIRED)
if ( OPENSSL_FOUND )
    message(STATUS "OpenSSL Found: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
endif()

link_libraries(${OPENSSL_LIBRARIES})

add_executable(cadir3 main.cpp Exceptions/FileHandlingException.h Exceptions/SetupCommandException.h Exceptions/CreateCacheDirectoryException.h Exceptions/CopyToCacheFailedException.h Exceptions/CleaningFailedException.h Exceptions/CopyFromCacheException.h Exceptions/LinkFromCacheException.h)

set(CMAKE_VERBOSE_MAKEFILE ON)