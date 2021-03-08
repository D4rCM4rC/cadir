//
// Created by Dirk Winkhaus on 08.03.21.
//

#ifndef CADIR3_COPYTOCACHEFAILEDEXCEPTION_H
#define CADIR3_COPYTOCACHEFAILEDEXCEPTION_H


#include "FileHandlingException.h"

class CopyToCacheFailedException : FileHandlingException {
    using FileHandlingException::FileHandlingException;
};


#endif //CADIR3_COPYTOCACHEFAILEDEXCEPTION_H
