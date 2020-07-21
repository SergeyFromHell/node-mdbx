#pragma once

#include <napi.h>
#include "mdbx.h"

class CppMdbx : public Napi::ObjectWrap<CppMdbx>
{
public:
    CppMdbx(const Napi::CallbackInfo&);
    Napi::Value Open(const Napi::CallbackInfo&);
    Napi::Value Close(const Napi::CallbackInfo&);

    Napi::Value BeginTransaction(const Napi::CallbackInfo&);
    Napi::Value CommitTransaction(const Napi::CallbackInfo&);
    Napi::Value AbortTransaction(const Napi::CallbackInfo&);
    
    Napi::Value Put(const Napi::CallbackInfo&);
    Napi::Value Get(const Napi::CallbackInfo&);
    Napi::Value Del(const Napi::CallbackInfo&);
    Napi::Value Has(const Napi::CallbackInfo&);

    static Napi::Function GetClass(Napi::Env);

    ~CppMdbx();

private:
    void _dbClose();

    std::string _dbPath;
    bool _readOnly = false;
    intptr_t _pageSize = 4096;

    MDBX_env *_dbEnv = NULL;
    MDBX_dbi _dbDbi = 0;
    MDBX_txn *_dbTxn = NULL;
    bool _opened = false;
};
