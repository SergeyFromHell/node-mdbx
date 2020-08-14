#pragma once

#include <napi.h>
#include "mdbx.h"

#include "db_env.h"

class CppMdbx : public Napi::ObjectWrap<CppMdbx>
{
public:
    CppMdbx(const Napi::CallbackInfo&);
    Napi::Value Close(const Napi::CallbackInfo&);

    // Napi::Value BeginTransaction(const Napi::CallbackInfo&);
    // Napi::Value CommitTransaction(const Napi::CallbackInfo&);
    // Napi::Value AbortTransaction(const Napi::CallbackInfo&);
    // Napi::Value HasTransaction(const Napi::CallbackInfo&);
    Napi::Value GetDbi(const Napi::CallbackInfo&);
    
    // Napi::Value Put(const Napi::CallbackInfo&);
    // Napi::Value Get(const Napi::CallbackInfo&);
    // Napi::Value Del(const Napi::CallbackInfo&);
    // Napi::Value Has(const Napi::CallbackInfo&);

    // Napi::Value FirstKey(const Napi::CallbackInfo&);
    // Napi::Value LastKey(const Napi::CallbackInfo&);
    // Napi::Value NextKey(const Napi::CallbackInfo&);
    // Napi::Value PrevKey(const Napi::CallbackInfo&);

    static Napi::Function GetClass(Napi::Env);

    ~CppMdbx();

private:
    void _dbClose();
    
    DbEnvPtr _dbEnvPtr;
    Napi::FunctionReference _cppDbiConstructor;
};
