#pragma once

#include <stdexcept>
#include "mdbx.h"

class DbException: public std::runtime_error {
public:
    template<typename T>
    DbException(T &&msg): std::runtime_error(std::forward<T>(msg)) {

    }
};

static void CheckMdbxResult(int rc) {
    if (rc != MDBX_SUCCESS)
        throw DbException(mdbx_strerror(rc));
};

