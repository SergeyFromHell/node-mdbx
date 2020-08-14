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

CppMdbx::~CppMdbx() {
    _dbClose();
}

Napi::Function CppMdbx::GetClass(Napi::Env env) {
    return DefineClass(env, "CppMdbx", {
        CppMdbx::InstanceMethod("close", &CppMdbx::Close),
        CppMdbx::InstanceMethod("getDbi", &CppMdbx::GetDbi),

        // CppMdbx::InstanceMethod("beginTransaction", &CppMdbx::BeginTransaction),
        // CppMdbx::InstanceMethod("commitTransaction", &CppMdbx::CommitTransaction),
        // CppMdbx::InstanceMethod("abortTransaction", &CppMdbx::AbortTransaction),
        // CppMdbx::InstanceMethod("hasTransaction", &CppMdbx::HasTransaction),

        // CppMdbx::InstanceMethod("put", &CppMdbx::Put),
        // CppMdbx::InstanceMethod("get", &CppMdbx::Get),
        // CppMdbx::InstanceMethod("del", &CppMdbx::Del),
        // CppMdbx::InstanceMethod("has", &CppMdbx::Has),

        // CppMdbx::InstanceMethod("firstKey", &CppMdbx::FirstKey),
        // CppMdbx::InstanceMethod("lastKey", &CppMdbx::LastKey),
        // CppMdbx::InstanceMethod("nextKey", &CppMdbx::NextKey),
        // CppMdbx::InstanceMethod("prevKey", &CppMdbx::PrevKey),
    });
}

void CppMdbx::_dbClose() {
    if (_dbEnvPtr)
        _dbEnvPtr->Close();
    _dbEnvPtr.reset();
}

Napi::Value CppMdbx::GetDbi(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (!_dbEnvPtr || !_dbEnvPtr->IsOpened())
        throw Napi::Error::New(env, "Closed.");

    Napi::Value nameValue = info[0];
    std::string name;
    if (!nameValue.IsNull() && !nameValue.IsUndefined())
        name = nameValue.ToString();

    MDBX_dbi dbi = _dbEnvPtr->OpenDbi(name);
    Napi::Value cppDbiValue = _cppDbiConstructor.New({});
    CppDbi *cppDbi = CppDbi::Unwrap(cppDbiValue.ToObject());
    cppDbi->Init(_dbEnvPtr, dbi);
    return cppDbiValue;
}

// Napi::Value CppMdbx::BeginTransaction(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     if (_dbTxn != NULL)
//         throw Napi::Error::New(env, "Multiple parallel transactions.");

//     int flags = 0;
//     if (_readOnly)
//         flags |= MDBX_RDONLY;
//     const int rc = mdbx_txn_begin(_dbEnv, NULL, flags, &_dbTxn);
//     CheckMdbxResult(rc, env);

//     return env.Undefined();
// }

// Napi::Value CppMdbx::CommitTransaction(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     _checkTransaction(env);

//     const int rc = mdbx_txn_commit(_dbTxn);
//     _dbTxn = NULL;
//     CheckMdbxResult(rc, env);

//     return env.Undefined();
// }

// Napi::Value CppMdbx::AbortTransaction(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     _checkTransaction(env);

//     const int rc = mdbx_txn_abort(_dbTxn);
//     _dbTxn = NULL;
//     CheckMdbxResult(rc, env);

//     return env.Undefined();
// }

// Napi::Value CppMdbx::HasTransaction(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     return Napi::Value::From(env, _dbTxn != NULL);
// }

// Napi::Value CppMdbx::Put(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     _checkTransaction(env);

//     ExtractBuffer(info[0], _keyBuffer);
//     ExtractBuffer(info[1], _valueBuffer);

//     MDBX_val key = CreateMdbxVal(_keyBuffer);
//     MDBX_val value = CreateMdbxVal(_valueBuffer);

//     const int rc = mdbx_put(_dbTxn, _dbDbi, &key, &value, 0);
//     CheckMdbxResult(rc, env);

//     return env.Undefined();
// }

// Napi::Value CppMdbx::Get(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     _checkTransaction(env);

//     ExtractBuffer(info[0], _keyBuffer);
//     MDBX_val key = CreateMdbxVal(_keyBuffer);
//     MDBX_val value;
//     const int rc = mdbx_get(_dbTxn, _dbDbi, &key, &value);

//     if (rc == MDBX_NOTFOUND)
//         return env.Undefined();

//     CheckMdbxResult(rc, env);

//     return Napi::Buffer<char>::Copy(env, (const char *)value.iov_base, value.iov_len);
// }

// Napi::Value CppMdbx::Del(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     _checkTransaction(env);

//     ExtractBuffer(info[0], _keyBuffer);
//     MDBX_val key = CreateMdbxVal(_keyBuffer);
//     const int rc = mdbx_del(_dbTxn, _dbDbi, &key, NULL);
//     CheckMdbxResult(rc, env);

//     return env.Undefined();
// }

// Napi::Value CppMdbx::Has(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     _checkTransaction(env);

//     ExtractBuffer(info[0], _keyBuffer);
//     MDBX_val key = CreateMdbxVal(_keyBuffer);
//     MDBX_val value;
//     const int rc = mdbx_get(_dbTxn, _dbDbi, &key, &value);

//     if (rc == MDBX_NOTFOUND)
//         return Napi::Value::From(env, false);

//     CheckMdbxResult(rc, env);

//     return Napi::Value::From(env, true);
// }

// Napi::Value CppMdbx::FirstKey(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     _checkTransaction(env);

//     MDBX_cursor *dbCur = NULL;

//     int rc = MDBX_SUCCESS;
//     try {
//         rc = mdbx_cursor_open(_dbTxn, _dbDbi, &dbCur);
//         CheckMdbxResult(rc, env);

//         //rc = mdbx_cursor_

//         mdbx_cursor_close(dbCur);
//         return env.Undefined();
//     } catch(...) {
//         if (dbCur != NULL)
//             mdbx_cursor_close(dbCur);
//         throw;
//     };

//     return env.Undefined();
// }

// Napi::Value CppMdbx::LastKey(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     _checkTransaction(env);

//     return env.Undefined();
// }

// Napi::Value CppMdbx::NextKey(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     _checkTransaction(env);

//     return env.Undefined();
// }

// Napi::Value CppMdbx::PrevKey(const Napi::CallbackInfo& info) {
//     Napi::Env env = info.Env();

//     _checkTransaction(env);

//     return env.Undefined();
// }

// void CppMdbx::_checkTransaction(Napi::Env env) {
//     if (_dbTxn == NULL)
//         throw Napi::Error::New(env, "No transaction started.");
// }

// void CppMdbx::_dbClose() {
//     if (!_opened)
//         return;

//     if (_dbTxn != NULL) {
//         mdbx_txn_abort(_dbTxn);
//         _dbTxn = NULL;
//     };

//     mdbx_dbi_close(_dbEnv, _dbDbi);
//     _dbDbi = 0;

//     mdbx_env_close(_dbEnv);
//     _dbEnv = NULL;
    
//     _opened = false;
// }
