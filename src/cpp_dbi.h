#pragma once

#include <napi.h>
#include "mdbx.h"
#include "db_env.h"

class CppDbi : public Napi::ObjectWrap<CppDbi>
{
public:
    CppDbi(const Napi::CallbackInfo & info): Napi::ObjectWrap<CppDbi>(info) {};

    inline static Napi::Function GetClass(Napi::Env env) {
        return DefineClass(env, "CppDbi", {

        });
    }

    inline void Init(const DbEnvPtr &dbEnvPtr, MDBX_dbi dbDbi) {
        _dbEnvPtr = dbEnvPtr;
        _dbDbi = dbDbi;
    }

    ~CppDbi() {}

private:
    DbEnvPtr _dbEnvPtr;
    MDBX_dbi _dbDbi = 0;
};
