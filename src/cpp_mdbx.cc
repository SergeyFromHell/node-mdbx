#include "cpp_mdbx.h"
#include "cpp_dbi.h"

#include <algorithm>
#include <iterator>
#include <string>

CppMdbx::CppMdbx(const Napi::CallbackInfo& info) : ObjectWrap(info) {
    Napi::Env env = info.Env();

    Napi::Object options = info[0].As<Napi::Object>();

    if (!options.Get("path").IsString())
        throw Napi::Error::New(env, "DB path is not a string.");
    const std::string dbPath = options.Get("path").ToString();
    if (dbPath.empty())
        throw Napi::Error::New(env, "DB path is empty.");
    
    const bool readOnly = options.Get("readOnly").ToBoolean();

    intptr_t pageSize = -1;
    if (options.Has("pageSize"))
        pageSize = options.Get("pageSize").ToNumber();

    const DbEnvParameters dbEnvParameters = {
        .dbPath = dbPath,
        .readOnly = readOnly,
        .pageSize = pageSize
    };
    _dbEnvPtr.reset(new DbEnv());
    _dbEnvPtr->Open(dbEnvParameters);

    _cppDbiConstructor = Napi::Persistent(CppDbi::GetClass(env));
}

Napi::Value CppMdbx::Close(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _dbClose();

    return env.Undefined();
}

void CppMdbx::_checkOpened(Napi::Env env) {
    if (!_dbEnvPtr || !_dbEnvPtr->IsOpened())
        throw Napi::Error::New(env, "Closed.");
}

CppMdbx::~CppMdbx() {
    _dbClose();
}

Napi::Function CppMdbx::GetClass(Napi::Env env) {
    return DefineClass(env, "CppMdbx", {
        CppMdbx::InstanceMethod("close", &CppMdbx::Close),
        CppMdbx::InstanceMethod("getDbi", &CppMdbx::GetDbi),

        CppMdbx::InstanceMethod("beginTransaction", &CppMdbx::BeginTransaction),
        CppMdbx::InstanceMethod("commitTransaction", &CppMdbx::CommitTransaction),
        CppMdbx::InstanceMethod("abortTransaction", &CppMdbx::AbortTransaction),
        CppMdbx::InstanceMethod("hasTransaction", &CppMdbx::HasTransaction),
    });
}

void CppMdbx::_dbClose() {
    if (_dbEnvPtr)
        _dbEnvPtr->Close();
    _dbEnvPtr.reset();
}

Napi::Value CppMdbx::GetDbi(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    _checkOpened(env);

    Napi::Value nameValue = info[0];
    std::string name;
    if (!nameValue.IsNull() && !nameValue.IsUndefined())
        name = nameValue.ToString();

    return wrapException(env, [&]() -> Napi::Value {
        MDBX_dbi dbi = _dbEnvPtr->OpenDbi(name);
        Napi::Value cppDbiValue = _cppDbiConstructor.New({});
        CppDbi *cppDbi = CppDbi::Unwrap(cppDbiValue.ToObject());
        cppDbi->Init(_dbEnvPtr, dbi);
        return cppDbiValue;
    });
}

Napi::Value CppMdbx::BeginTransaction(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    _checkOpened(env);
    _dbEnvPtr->BeginTransaction();
    return env.Undefined();
}

Napi::Value CppMdbx::CommitTransaction(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    _checkOpened(env);
    _dbEnvPtr->CommitTransaction();
    return env.Undefined();
}

Napi::Value CppMdbx::AbortTransaction(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    _checkOpened(env);
    _dbEnvPtr->AbortTransaction();
    return env.Undefined();
}

Napi::Value CppMdbx::HasTransaction(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    _checkOpened(env);
    const bool hasTransaction = _dbEnvPtr->HasTransaction();
    return Napi::Value::From(env, hasTransaction);
}
