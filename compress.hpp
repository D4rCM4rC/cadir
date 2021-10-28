#pragma once //"exitCodeEnum.hpp"

#include <iostream>
#include <fstream>
#include <archive.h>
#include <archive_entry.h>
#include <vector>
#include "fileSystem.hpp"
#include "exitCodeEnum.hpp"
#include "Exceptions/GzipWriteReadException.h"

namespace compress {
    const int bufferSize = 1024 * 1024 * 4;

    void write_archive(const std::string &rootPath, const char *outname, std::vector<std::string> files) {
        struct archive *archive;
        struct archive_entry *archiveEntry;
        struct stat st;
        int len;
        std::ifstream fileStream;
        char buffer[bufferSize];

        archive = archive_write_new();
        if (
                archive_write_add_filter_gzip(archive) ||
                archive_write_set_format_pax_restricted(archive) ||
                archive_write_open_filename(archive, outname) ==1 )
            throw (GzipWriteReadException("GZip Exception", ExitCode::gzipException));

        for (auto iterator = files.begin(); iterator != files.end(); iterator++) {
            std::string fileName = *iterator;
            std::string relativeFileName = std::filesystem::relative(fileName, rootPath);

            stat(fileName.c_str(), &st);
            archiveEntry = archive_entry_new();

            archive_entry_set_pathname(archiveEntry, relativeFileName.c_str());
            archive_entry_copy_stat(archiveEntry, &st);

            archive_write_header(archive, archiveEntry);

            fileStream.open(fileName, std::ifstream::binary | std::ifstream::out);

            while (fileStream.good()) {
                fileStream.read(buffer, sizeof(buffer));
                ssize_t written = archive_write_data(archive, buffer, (size_t) fileStream.gcount());
            }

            fileStream.close();
            archive_entry_free(archiveEntry);

        }

        archive_write_close(archive);
        archive_write_free(archive);
    }

    static int copy_data(struct archive *ar, struct archive *aw) {
        int r;
        const void *buff;
        size_t size;
        off_t offset;

        for (;;) {
            r = archive_read_data_block(ar, &buff, &size, &offset);
            if (r == ARCHIVE_EOF)
                return (ARCHIVE_OK);
            if (r < ARCHIVE_OK)
                return (r);
            r = archive_write_data_block(aw, buff, size, offset);
            if (r < ARCHIVE_OK) {
                fprintf(stderr, "%s\n", archive_error_string(aw));
                return (r);
            }
        }
    }

    static int extract(const char *filename) {
        struct archive *a;
        struct archive *ext;
        struct archive_entry *entry;
        int flags;
        int r;

        /* Select which attributes we want to restore. */
        flags = ARCHIVE_EXTRACT_TIME;
        flags |= ARCHIVE_EXTRACT_PERM;
        flags |= ARCHIVE_EXTRACT_ACL;
        flags |= ARCHIVE_EXTRACT_FFLAGS;

        a = archive_read_new();

        archive_read_support_filter_all(a);
        archive_read_support_format_all(a);

        ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, flags);
        archive_write_disk_set_standard_lookup(ext);

        if ((r = archive_read_open_filename(a, filename, 10240))) {
            fprintf(stderr, "%s\n", archive_error_string(a));

            return 1;
        }

        for (;;) {
            r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF)
                break;
            if (r < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(a));
            if (r < ARCHIVE_WARN)
                return 1;
            r = archive_write_header(ext, entry);
            if (r < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(ext));
            else if (archive_entry_size(entry) > 0) {
                r = copy_data(a, ext);
                if (r < ARCHIVE_OK) {
                    fprintf(stderr, "%s\n", archive_error_string(ext));
                }
                if (r < ARCHIVE_WARN)
                    return 1;
            }
            r = archive_write_finish_entry(ext);
            if (r < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(ext));
            if (r < ARCHIVE_WARN)
                return 1;
        }

        archive_read_close(a);
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);

        return 0;
    }
}