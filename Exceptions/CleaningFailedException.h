//
// Created by Dirk Winkhaus on 08.03.21.
//

#ifndef CADIR3_CLEANINGFAILEDEXCEPTION_H
#define CADIR3_CLEANINGFAILEDEXCEPTION_H


#include "FileHandlingException.h"

class CleaningFailedException : FileHandlingException {
    using FileHandlingException::FileHandlingException;
};


#endif //CADIR3_CLEANINGFAILEDEXCEPTION_H
