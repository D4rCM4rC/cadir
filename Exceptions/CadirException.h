//
// Created by Dirk Winkhaus on 08.03.21.
//

#ifndef CADIR3_CADIREXCEPTION_H
#define CADIR3_CADIREXCEPTION_H

#include <exception>
#include <iostream>

class CadirException: virtual public std::exception {
protected:

    int errorCode;
    std::string errorMessage;

public:

    explicit
    CadirException(const std::string &msg, const int &err_num = 0):
            errorCode(err_num),
            errorMessage(msg)
    {}

    virtual ~CadirException() throw () {}

    virtual const char* what() const throw () {
        return errorMessage.c_str();
    }

    virtual int getErrorCode() const throw() {
        return errorCode;
    }
};


#endif //CADIR3_CADIREXCEPTION_H
