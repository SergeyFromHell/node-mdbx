#pragma once

#include <napi.h>
#include "mdbx.h"

#include "db_env.h"

class CppMdbx : public Napi::ObjectWrap<CppMdbx>
{
public:
    CppMdbx(const Napi::CallbackInfo&);
    Napi::Value Close(const Napi::CallbackInfo&);

    Napi::Value BeginTransaction(const Napi::CallbackInfo&);
    Napi::Value CommitTransaction(const Napi::CallbackInfo&);
    Napi::Value AbortTransaction(const Napi::CallbackInfo&);
    Napi::Value HasTransaction(const Napi::CallbackInfo&);
    Napi::Value GetDbi(const Napi::CallbackInfo&);
    
    static Napi::Function GetClass(Napi::Env);

    ~CppMdbx();

private:
    void _dbClose();
    void _checkOpened(Napi::Env);
    
    DbEnvPtr _dbEnvPtr;
    Napi::FunctionReference _cppDbiConstructor;
};
