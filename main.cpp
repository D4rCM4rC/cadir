#include <cstring>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "openssl/md5.h"

const int currentWorkingDirectoryArgument = 0;
const int sourceCopyDirectoryArgument = 1;
const int sourceHashFileArgument = 2;
const int targetCacheDirectoryArgument = 3;
const int commandWorkingDirectoryArgument = 4;
const int commandArgument = 5;
const int copyModeArgument = 6;

std::string generateCommand(char *const *argument_list);

void display_help() {
    std::cout << "Usage: \n";
    std::cout
            << "cadir DIRECTORY_TO_BE_CACHED FILE_FOR_IDENTIFY_CACHE DIRECTORY_FOR_CACHES COMMAND_WORKING_DIRECTORY COMMAND [COPY_MODE] \n";
    std::cout << "cadir test/source/vendor test/source/composer.lock test/cache test/source/ \"composer install\"\n";
    std::cout
            << "cadir test/source/vendor test/source/composer.lock test/cache test/source/ \"composer install\" copy\n";
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

bool isAbsolutePath(const std::string &directory) {
    return directory.front() == '/';
}

std::string generateCommand(char *const *argument_list) {
    std::string commandString = "cd ";
    commandString
            .append(argument_list[commandWorkingDirectoryArgument])
            .append("; ")
            .append(argument_list[commandArgument]);
    return commandString;
}

std::string removeLastStringAfterSlash(const std::string &content) {
    return content.substr(0, (content.rfind('/') + 1));
};

int main(int argument_count, char **argument_list) {
    std::cout << argument_list[0];
    if (argument_count < 6 || argument_count > 7) {
        display_help();

        return 1;
    }

    bool copyMode = false;

    if (argument_count == 7 && strcmp(argument_list[copyModeArgument], "copy") == 0) {
        copyMode = true;
    }

    std::string directory;

    try {
        directory = generateMd5FromString(getFileContents(argument_list[sourceHashFileArgument]));
    } catch (std::invalid_argument &exception) {
        std::cout << exception.what() << argument_list[sourceHashFileArgument] << "\n";
        return 2;
    }
    
    std::string currentWorkingDirectoryPath(argument_list[currentWorkingDirectoryArgument]);
    currentWorkingDirectoryPath = removeLastStringAfterSlash(currentWorkingDirectoryPath);

    std::string cacheDirectory(argument_list[targetCacheDirectoryArgument]);
    std::string sourceDirectoryPath(argument_list[sourceCopyDirectoryArgument]);
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

        std::string fromPath = targetDirectoryPath;
        if (!isAbsolutePath(targetDirectoryPath)) {
            fromPath = currentWorkingDirectoryPath.append(targetDirectoryPath);
        }
        if (copyMode) {
            std::filesystem::copy(targetDirectoryPath, sourceDirectoryPath, copyOptions);
        }
        std::filesystem::create_symlink(fromPath, sourceDirectoryPath);
    }

    return 0;
}
