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
    gzipException = 10,
};