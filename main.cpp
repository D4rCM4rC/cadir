#include <cstring>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <array>
#include "openssl/md5.h"
#include "CLI11.hpp"

const int currentWorkingDirectoryArgument = 0;
bool verbose = false;

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

std::string getFileContents(const std::string &fileName) {
    std::ifstream file(fileName);

    if (!file.good()) {
        throw std::invalid_argument("Cannot access file");
    }

    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

std::string generatePath(std::string &directory, const std::string &name) {
    if (directory.back() != '/') {
        directory.append("/");
    }

    return directory.append(name);
}

bool isAbsolutePath(const std::string &directory) {
    return directory.front() == '/';
}

std::string generateCommand(const std::string &workingDirectory, const std::string &command) {
    std::string commandString = "cd ";
    commandString
            .append(workingDirectory)
            .append("; ")
            .append(command);
    return commandString;
}

std::string removeLastStringAfterSlash(const std::string &content) {
    return content.substr(0, (content.rfind('/') + 1));
};

std::string executeCommand(const std::string &command) {
    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe) throw std::runtime_error("popen() failed!");

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

void trace(const std::string &log, bool const force = false) {
    if (!force && ! verbose) {
        return;
    }

    std::cout << log << "\n";
}

int main(int argumentCount, char **argumentList) {
    std::string identityFile;
    std::string commandWorkingDirectory;
    std::string setupCommand;
    std::string finalizeCommand;
    std::string targetCacheDirectoryPath;
    std::string cacheSource;
    std::string currentWorkingDirectoryPath(removeLastStringAfterSlash(argumentList[currentWorkingDirectoryArgument]));
    std::string generatedHashTargetDirectory;
    bool linkCache = false;

    // new arguments: --cache-source="test/source/vendor" --identity-file="test/source/composer.lock" --cache-destination="test/cache" --command-working-directory="test/source/" --command="composer install --ignore-platform-reqs" --finalize-command="composer dump-autoload"
    CLI::App app{"App description"};


    app.add_option("-s,--cache-source", cacheSource, "The directory which should be cached");
    app.add_option("-i,--identity-file", identityFile, "File which shows differences");
    app.add_option("-d,--cache-destination", targetCacheDirectoryPath, "The directory where the cache is stored");
    app.add_option("-w,--command-working-directory", commandWorkingDirectory,
                   "Working directory where the setup command is called from");
    app.add_option("-c,--command", setupCommand, "Argument which is called if cache is not found");
    app.add_option("-f,--finalize-command", finalizeCommand,
                   "[optional] Command which is called after cache is regenerated, linked or copied");
    app.add_flag("-v,--verbose", verbose, "Show verbose output");
    app.add_flag("-l,--link", linkCache, "Link cache instead of copy");

    try {
        app.parse(argumentCount, argumentList);
    } catch (const std::exception& ex) {
        trace(ex.what(), true);
        trace(app.help(), true);

        return 1;
    }

    std::cout << app.help();

    std::string commandString;
    std::string targetDirectoryPath;

    try {
        generatedHashTargetDirectory = generateMd5FromString(getFileContents(identityFile));
    } catch (std::invalid_argument &exception) {
        std::cout << exception.what() << identityFile << "\n";
        return 2;
    }

    targetDirectoryPath = generatePath(targetCacheDirectoryPath, generatedHashTargetDirectory);

    trace("Identity file is: " + generatedHashTargetDirectory);

    const auto copyOptions = std::experimental::filesystem::copy_options::recursive |
                             std::experimental::filesystem::copy_options::overwrite_existing |
                             std::experimental::filesystem::copy_options::copy_symlinks;

    if (!std::experimental::filesystem::exists(targetDirectoryPath)) {
        trace("No cache exists");
        commandString = generateCommand(commandWorkingDirectory, setupCommand);
        trace("Execute: " + commandString);
        executeCommand(commandString);
        trace("Create cache directory: " + targetDirectoryPath);
        std::experimental::filesystem::create_directories(targetDirectoryPath);
        trace("Copy data from " + cacheSource + " to " + targetDirectoryPath);
        std::experimental::filesystem::copy(cacheSource, targetDirectoryPath, copyOptions);
    } else {
        trace("Cache found");
        if (std::experimental::filesystem::exists(cacheSource)) {
            std::experimental::filesystem::remove_all(cacheSource);
        }

        std::string fromPath = targetDirectoryPath;
        if (!linkCache) {
            trace("Copy data from " + targetDirectoryPath + " to " + cacheSource);
            std::experimental::filesystem::copy(targetDirectoryPath, cacheSource, copyOptions);
            commandString = generateCommand(commandWorkingDirectory, finalizeCommand);
            trace("Execute: " + commandString);
            executeCommand(commandString);
        } else {
            if (!isAbsolutePath(targetDirectoryPath)) {
                fromPath = currentWorkingDirectoryPath.append(targetDirectoryPath);
            }
            trace("Create link from " + fromPath + " to " + cacheSource);
            std::experimental::filesystem::create_symlink(fromPath, cacheSource);
        }
    }

    return 0;
}
