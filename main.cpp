#include <cstring>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <array>
#include "openssl/md5.h"
#include "CLI11.hpp"
#include "Exceptions/SetupCommandException.h"
#include "Exceptions/CreateCacheDirectoryException.h"
#include "Exceptions/CopyToCacheFailedException.h"
#include "Exceptions/CleaningFailedException.h"
#include "Exceptions/CopyFromCacheException.h"
#include "Exceptions/LinkFromCacheException.h"

enum ExitCode {
    ok = 0,
    argumentParsingFailed = 1,
    identityFileFailed = 2,
    setupCommandFailed = 3,
    finalizeCommandFailed = 4,
    copyToCacheFailed = 5,
    copyFromCacheFailed = 6,
    createSymLinkFailed = 7,
    cleaningFailed = 8,
    createCacheDirectoriesFailed = 9,
};

const int currentWorkingDirectoryArgument = 0;
const auto defaultCopyOptions = std::experimental::filesystem::copy_options::recursive |
                                std::experimental::filesystem::copy_options::overwrite_existing |
                                std::experimental::filesystem::copy_options::copy_symlinks;

bool verbose = false;

void showHelpText(const std::string &help);

std::string generateMd5FromString(const std::string &content);

std::string getFileContents(const std::string &fileName);

std::string generatePath(std::string &directory, const std::string &name);

bool isAbsolutePath(const std::string &directory);

std::string generateCommand(const std::string &workingDirectory, const std::string &command);

std::string removeLastStringAfterSlash(const std::string &content);

int executeCommand(std::string command);

void trace(const std::string &log, bool const &force = false);

void trace(bool const &force = false);

void createCache(
        const std::string &setupCommand,
        const std::string &cacheSource,
        const std::string &commandString,
        const std::string &targetDirectoryPath,
        const std::experimental::filesystem::copy_options &copyOptions
);

void loadFromCache(
        const std::string &cacheSource,
        const std::string &currentWorkingDirectoryPath,
        bool linkCache,
        const std::string &commandString,
        const std::string &targetDirectoryPath,
        const std::experimental::filesystem::copy_options &copyOptions
);

int main(int argumentCount, char **argumentList) {
    try {
        std::string identityFile;
        std::string commandWorkingDirectory;
        std::string setupCommand;
        std::string finalizeCommand;
        std::string targetCacheDirectoryPath;
        std::string cacheSource;
        std::string currentWorkingDirectoryPath(
                removeLastStringAfterSlash(argumentList[currentWorkingDirectoryArgument]));
        std::string generatedHashTargetDirectory;
        bool linkCache = false;
        bool showHelp = false;

        CLI::App app{"cadir description", "cadir"};
        app.remove_option(app.get_help_ptr());

        app.add_option("--cache-source", cacheSource, "The directory which should be cached");
        app.add_option("--identity-file", identityFile, "File which shows differences");
        app.add_option("--cache-destination", targetCacheDirectoryPath, "The directory where the cache is stored");
        app.add_option("--command-working-directory", commandWorkingDirectory,
                       "Working directory where the setup command is called from");
        app.add_option("--setup", setupCommand, "Argument which is called if cache is not found");
        app.add_option("--finalize", finalizeCommand,
                       "[optional] Command which is called after cache is regenerated, linked or copied");
        app.add_flag("-v,--verbose", verbose, "Show verbose output");
        app.add_flag("-l,--link", linkCache, "Link cache instead of copy");
        app.add_flag("-h,--help", showHelp, "Show help");

        try {
            app.parse(argumentCount, argumentList);

            // validation of mandatory arguments
            app.get_option("--cache-source");
            app.get_option("--identity-file");
            app.get_option("--cache-destination");
            app.get_option("--command-working-directory");
            app.get_option("--setup");

        } catch (const std::exception &ex) {
            trace(ex.what(), true);
            trace(true);
            showHelpText(app.help());

            return ExitCode::argumentParsingFailed;
        }

        if (showHelp) {
            showHelpText(app.help());

            return 0;
        }

        std::string commandString;
        std::string targetDirectoryPath;

        try {
            generatedHashTargetDirectory = generateMd5FromString(getFileContents(identityFile));
        } catch (std::invalid_argument &exception) {
            trace(exception.what());
            trace(identityFile);

            return ExitCode::identityFileFailed;
        }

        targetDirectoryPath = generatePath(targetCacheDirectoryPath, generatedHashTargetDirectory);

        trace("Identity file is: " + generatedHashTargetDirectory);


        if (!std::experimental::filesystem::exists(targetDirectoryPath)) {
            trace("No cache exists");
            commandString = generateCommand(commandWorkingDirectory, setupCommand);

            createCache(
                    setupCommand,
                    cacheSource,
                    commandString,
                    targetDirectoryPath,
                    defaultCopyOptions
            );
        } else {
            commandString =
                    (finalizeCommand != "")
                    ? generateCommand(
                            commandWorkingDirectory,
                            finalizeCommand
                    )
                    : "";

            loadFromCache(
                    cacheSource,
                    currentWorkingDirectoryPath,
                    linkCache,
                    commandString,
                    targetDirectoryPath,
                    defaultCopyOptions
            );
        }

        return ExitCode::ok;
    } catch (CadirException &exception) {
        trace(exception.what());
        return exception.getErrorCode();
    }
}

void showHelpText(const std::string &helpText) {
    trace(helpText, true);
    trace(true);
    trace("Exit Codes", true);
    trace("==========", true);
    trace("0 = Successfully executed", true);
    trace("1 = Wrong usage of arguments", true);
    trace("2 = Identity file error (not found/no rights)", true);
    trace("3 = Setup command failed", true);
    trace("4 = Finalize command failed", true);
    trace("5 = Cannot copy to cache directoy", true);
    trace("6 = Cannot copy from cache directoy", true);
    trace("7 = Cannot create link from cache", true);
    trace("8 = Removing existing cache folder failed", true);
    trace("9 = Cannot create cache directories", true);
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

int executeCommand(std::string command) {
    if (verbose) {
        FILE *c = popen(command.c_str(), "r");

        return pclose(c);
    }

    return system(command.append(" &> /dev/null").c_str());
}


void trace(bool const &force) {
    if (!force && !verbose) {
        return;
    }

    std::cout << "\n";
}

void trace(const std::string &log, bool const &force) {
    if (!force && !verbose) {
        return;
    }

    std::cout << log;

    trace(force);
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

void createCache(
        const std::string &setupCommand,
        const std::string &cacheSource,
        const std::string &commandString,
        const std::string &targetDirectoryPath,
        const std::experimental::filesystem::copy_options &copyOptions
) {
    trace("Execute: " + commandString);
    if (executeCommand(commandString)) {
        throw (SetupCommandException("Setup command failed", ExitCode::setupCommandFailed));
    }
    try {
        trace("Create cache directory: " + targetDirectoryPath);
        std::experimental::filesystem::create_directories(targetDirectoryPath);
    } catch (...) {
        throw (CreateCacheDirectoryException("Create cache directories failed",
                                             ExitCode::createCacheDirectoriesFailed));
    }
    try {
        trace("Copy data from " + cacheSource + " to " + targetDirectoryPath);
        std::experimental::filesystem::copy(cacheSource, targetDirectoryPath, copyOptions);
    } catch (...) {
        trace("Copy to cache failed");
        throw (CopyToCacheFailedException("Copy to cache failed", ExitCode::copyToCacheFailed));
    }
}

void loadFromCache(
        const std::string &cacheSource,
        const std::string &currentWorkingDirectoryPath,
        const bool linkCache,
        const std::string &commandString,
        const std::string &targetDirectoryPath,
        const std::experimental::filesystem::copy_options &copyOptions
) {
    trace("Cache found");
    try {
        if (std::experimental::filesystem::exists(cacheSource)) {
            std::experimental::filesystem::remove_all(cacheSource);
        }
    } catch (...) {
        throw (CleaningFailedException("Cleaning for cache regeneration failed", ExitCode::cleaningFailed));
    }
    std::string fromPath = targetDirectoryPath;
    if (!linkCache) {
        try {
            trace("Copy data from " + targetDirectoryPath + " to " + cacheSource);
            std::experimental::filesystem::copy(targetDirectoryPath, cacheSource, copyOptions);
        } catch (...) {
            throw (CopyFromCacheException("Copy from cache failed", ExitCode::copyFromCacheFailed));
        }

        if (commandString != "") {
            trace("Execute: " + commandString);

            if (executeCommand(commandString)) {
                throw (SetupCommandException("Finalize command failed", ExitCode::finalizeCommandFailed));
            }
        }
    } else {
        if (!isAbsolutePath(targetDirectoryPath)) {
            fromPath = currentWorkingDirectoryPath;
            fromPath.append(targetDirectoryPath);
        }
        trace("Create link from " + fromPath + " to " + cacheSource);
        try {
            std::experimental::filesystem::create_symlink(fromPath, cacheSource);
        } catch (...) {
            throw (LinkFromCacheException("Cannot create symlink", ExitCode::createSymLinkFailed));
        }
    }
}
