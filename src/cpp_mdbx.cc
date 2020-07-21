#include "cpp_mdbx.h"

void checkMdbxResult(int rc, Napi::Env &env) {
    if (rc != MDBX_SUCCESS)
        throw Napi::Error::New(env, mdbx_strerror(rc));
}

CppMdbx::CppMdbx(const Napi::CallbackInfo& info) : ObjectWrap(info) {
    Napi::Env env = info.Env();

    Napi::Object options = info[0].As<Napi::Object>();
    if (!options.Get("path").IsString())
        throw Napi::Error::New(env, "DB path is not a string.");
    _dbPath = options.Get("path").ToString();
    if (_dbPath.empty())
        throw Napi::Error::New(env, "DB path is empty.");
    
    if (options.Get("readOnly").ToBoolean())
        _readOnly = true;

    if (options.Has("pageSize"))
        _pageSize = options.Get("pageSize").ToNumber();

    _opened = false;
}

Napi::Value CppMdbx::Open(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (_opened)
        throw Napi::Error::New(env, "Already opened.");
    
    int rc = MDBX_SUCCESS;
    unsigned flags = 0;
    MDBX_env *dbEnv = NULL;
    MDBX_dbi dbDbi = 0;
    MDBX_txn *dbTxn = NULL;

    try {
        rc = mdbx_env_create(&dbEnv);
        checkMdbxResult(rc, env);

        rc = mdbx_env_set_geometry(dbEnv, -1, -1, -1, -1, -1, _pageSize);
        checkMdbxResult(rc, env);

        flags = MDBX_ACCEDE | MDBX_LIFORECLAIM | MDBX_NOMETASYNC | MDBX_SAFE_NOSYNC;
        if (_readOnly)
            flags |= MDBX_RDONLY;
        rc = mdbx_env_open(dbEnv, _dbPath.c_str(), flags, 0666);
        checkMdbxResult(rc, env);

        flags = 0;
        if (_readOnly)
            flags |= MDBX_RDONLY;
        rc = mdbx_txn_begin(dbEnv, NULL, flags, &dbTxn);
        checkMdbxResult(rc, env);

        flags = MDBX_CREATE;
        if (_readOnly)
            flags = 0;
        rc = mdbx_dbi_open(dbTxn, NULL, 0, &dbDbi);
        checkMdbxResult(rc, env);

        rc = mdbx_txn_commit(dbTxn);
        checkMdbxResult(rc, env);

        _dbEnv = dbEnv;
        _dbDbi = dbDbi;

        _opened = true;

        return env.Undefined();
    } catch(...) {
        if (dbTxn)
            mdbx_txn_abort(dbTxn);
        if (dbDbi)
            mdbx_dbi_close(dbEnv, dbDbi);
        if (env)
            mdbx_env_close(dbEnv);
        throw;
    };
}

Napi::Value CppMdbx::Close(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    _dbClose();
    return env.Undefined();
}

Napi::Value CppMdbx::BeginTransaction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (_dbTxn)
        throw Napi::Error::New(env, "Multiple parallel transactions.");

    int flags = 0;
    if (_readOnly)
        flags |= MDBX_RDONLY;
    const int rc = mdbx_txn_begin(_dbEnv, NULL, flags, &_dbTxn);
    checkMdbxResult(rc, env);

    return env.Undefined();
}

Napi::Value CppMdbx::CommitTransaction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (!_dbTxn)
        throw Napi::Error::New(env, "No transaction started.");

    const int rc = mdbx_txn_commit(_dbTxn);
    _dbTxn = NULL;
    checkMdbxResult(rc, env);

    return env.Undefined();
}

Napi::Value CppMdbx::AbortTransaction(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (!_dbTxn)
        throw Napi::Error::New(env, "No transaction started.");

    const int rc = mdbx_txn_abort(_dbTxn);
    _dbTxn = NULL;
    checkMdbxResult(rc, env);

    return env.Undefined();
}

Napi::Value CppMdbx::Put(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    return env.Undefined();
}

Napi::Value CppMdbx::Get(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    return env.Undefined();
}

Napi::Value CppMdbx::Del(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    return env.Undefined();
}

Napi::Value CppMdbx::Has(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    return env.Undefined();
}

Napi::Function CppMdbx::GetClass(Napi::Env env) {
    return DefineClass(env, "CppMdbx", {
        CppMdbx::InstanceMethod("open", &CppMdbx::Open),
        CppMdbx::InstanceMethod("close", &CppMdbx::Close),

        CppMdbx::InstanceMethod("begin", &CppMdbx::BeginTransaction),
        CppMdbx::InstanceMethod("commit", &CppMdbx::CommitTransaction),
        CppMdbx::InstanceMethod("abort", &CppMdbx::AbortTransaction),

        CppMdbx::InstanceMethod("put", &CppMdbx::Put),
        CppMdbx::InstanceMethod("get", &CppMdbx::Get),
        CppMdbx::InstanceMethod("del", &CppMdbx::Del),
        CppMdbx::InstanceMethod("has", &CppMdbx::Has),
    });
}

void CppMdbx::_dbClose() {
    if (!_opened)
        return;

    if (_dbTxn) {
        mdbx_txn_abort(_dbTxn);
        _dbTxn = NULL;
    };

    mdbx_dbi_close(_dbEnv, _dbDbi);
    _dbDbi = 0;

    mdbx_env_close(_dbEnv);
    _dbEnv = NULL;
    
    _opened = false;
}

CppMdbx::~CppMdbx() {
    _dbClose();
}