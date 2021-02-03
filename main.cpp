#include <cstring>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "openssl/md5.h"

const int sourceCopyDirectory = 1;
const int sourceHashFile = 2;
const int targetCacheDirectory = 3;
const int commandWorkingDirectory = 4;
const int command = 5;

std::string generateCommand(char *const *argument_list);

void display_help() {
    std::cout << "Usage: \n";
    std::cout
            << "cadir [DIRECTORY_TO_BE_CACHED] [FILE_FOR_IDENTIFY_CACHE] [DIRECTORY_FOR_CACHES] [COMMAND_WORKING_DIRECTORY] [COMMAND] \n";
    std::cout << "cadir test/source/vendor test/source/composer.lock test/cache test/source/ \"composer install\" \n";
}

std::string generateMd5FromString(const std::string &content) {
    MD5_CTX context;
    unsigned char md5Data[16];
    char buffer[3];

    MD5_Init(&context);
    MD5_Update(&context, content.c_str(), strlen(content.c_str()));
    MD5_Final(md5Data, &context);

    std::string md5String;
    for (int i = 0; i < 16; i++) {
        sprintf(buffer, "%02x", md5Data[i]);
        md5String.append(buffer);
    }

    return md5String;
}

std::string getFileContents(char *fileName) {
    std::ifstream file(fileName);

    if (!file.good()) {
        throw std::invalid_argument("Cannot access file");
    }

    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

std::string generatePath(std::string directory, const std::string &name) {
    if (directory.back() != '/') {
        directory.append("/");
    }

    return directory.append(name);
}

int main(int argument_count, char **argument_list) {
    if (argument_count != 6) {
        display_help();

        return 1;
    }
    std::string directory;

    try {
        directory = generateMd5FromString(getFileContents(argument_list[sourceHashFile]));
    } catch (std::invalid_argument &exception) {
        std::cout << exception.what() << argument_list[sourceHashFile];
        return 2;
    }


    std::string cacheDirectory(argument_list[targetCacheDirectory]);
    std::string sourceDirectoryPath(argument_list[sourceCopyDirectory]);
    std::string targetDirectoryPath = generatePath(cacheDirectory, directory);
    const auto copyOptions = std::filesystem::copy_options::recursive |
                             std::filesystem::copy_options::overwrite_existing;


    if (!std::filesystem::exists(targetDirectoryPath)) {
        std::string commandString = generateCommand(argument_list);
        popen(commandString.c_str(), "rw");
        std::filesystem::copy(sourceDirectoryPath, targetDirectoryPath, copyOptions);
    } else {
        if (std::filesystem::exists(sourceDirectoryPath)) {
            std::filesystem::remove_all(sourceDirectoryPath);
        }

        std::filesystem::copy(targetDirectoryPath, sourceDirectoryPath, copyOptions);
    }

    return 0;
}

std::string generateCommand(char *const *argument_list) {
    std::string commandString = "cd ";
    commandString
            .append(argument_list[commandWorkingDirectory])
            .append("; ")
            .append(argument_list[command]);
    return commandString;
}
