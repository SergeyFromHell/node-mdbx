#include "cpp_dbi.h"
#include "utils.h"

CppDbi::CppDbi(const Napi::CallbackInfo & info): Napi::ObjectWrap<CppDbi>(info) {};

Napi::Function CppDbi::GetClass(Napi::Env env) {
    return DefineClass(env, "CppDbi", {
        CppDbi::InstanceMethod("isStale", &CppDbi::IsStale),

        CppDbi::InstanceMethod("put", &CppDbi::Put),
        CppDbi::InstanceMethod("get", &CppDbi::Get),
        CppDbi::InstanceMethod("getValuesCount", &CppDbi::GetValuesCount),
        CppDbi::InstanceMethod("hasDup", &CppDbi::HasDup),
        CppDbi::InstanceMethod("del", &CppDbi::Del),
        CppDbi::InstanceMethod("delDup", &CppDbi::DelDup),
        CppDbi::InstanceMethod("has", &CppDbi::Has),

        CppDbi::InstanceMethod("first", &CppDbi::FirstKey),
        CppDbi::InstanceMethod("last", &CppDbi::LastKey),
        CppDbi::InstanceMethod("next", &CppDbi::NextKey),
        CppDbi::InstanceMethod("prev", &CppDbi::PrevKey),
        CppDbi::InstanceMethod("lowerBound", &CppDbi::LowerBoundKey),
    });
}

void CppDbi::Init(const DbEnvPtr &dbEnvPtr, MDBX_dbi dbDbi, const std::string &name) {
    _dbEnvPtr = dbEnvPtr;
    _dbDbi = dbDbi;
    _name = name;
}

Napi::Value CppDbi::IsStale(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);

    bool isStale = _dbEnvPtr->IsStale(_name, _dbDbi);

    return Napi::Value::From(env, isStale);
}

Napi::Value CppDbi::Put(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);

    ExtractBuffer(info[0], _keyBuffer);
    ExtractBuffer(info[1], _valueBuffer);

    return wrapException(env, [&] () {
        MDBX_val key = CreateMdbxVal(_keyBuffer);
        MDBX_val value = CreateMdbxVal(_valueBuffer);

        const int rc = mdbx_put(_dbEnvPtr->GetTransaction(), _dbDbi, &key, &value, MDBX_UPSERT);
        CheckMdbxResult(rc);

        return env.Undefined();
    });
}

Napi::Value CppDbi::Get(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);

    ExtractBuffer(info[0], _keyBuffer);

    return wrapException(env, [&] () {
        MDBX_val key = CreateMdbxVal(_keyBuffer);
        MDBX_val value;

        const int rc = mdbx_get(_dbEnvPtr->GetTransaction(), _dbDbi, &key, &value);
        if (rc == MDBX_NOTFOUND)
            return env.Undefined();
        CheckMdbxResult(rc);

        Napi::Buffer<char> result = Napi::Buffer<char>::Copy(env, (const char *)value.iov_base, value.iov_len);

        return _outValue(result);
    });
}

Napi::Value CppDbi::GetValuesCount(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);

    ExtractBuffer(info[0], _keyBuffer);

    return wrapException(env, [&] () {
        MDBX_val key = CreateMdbxVal(_keyBuffer);
        MDBX_val value;
        size_t count;

        const int rc = mdbx_get_ex(_dbEnvPtr->GetTransaction(), _dbDbi, &key, &value, &count);
        if (rc == MDBX_NOTFOUND)
            return env.Undefined();
        printf("%ld\n", count);
        CheckMdbxResult(rc);

        return Napi::Value::From(env, count);
    });
}

Napi::Value CppDbi::Del(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);

    ExtractBuffer(info[0], _keyBuffer);

    return wrapException(env, [&] () {
        MDBX_val key = CreateMdbxVal(_keyBuffer);

        const int rc = mdbx_del(_dbEnvPtr->GetTransaction(), _dbDbi, &key, NULL);
        if (rc == MDBX_NOTFOUND)
            return Napi::Value::From(env, false);
        CheckMdbxResult(rc);

        return Napi::Value::From(env, true);
    });
}

Napi::Value CppDbi::DelDup(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);

    ExtractBuffer(info[0], _keyBuffer);
    ExtractBuffer(info[1], _dupKeyBuffer);

    return wrapException(env, [&] () {
        MDBX_val key = CreateMdbxVal(_keyBuffer);
        MDBX_val dupKey = CreateMdbxVal(_dupKeyBuffer);

        const int rc = mdbx_del(_dbEnvPtr->GetTransaction(), _dbDbi, &key, &dupKey);
        if (rc == MDBX_NOTFOUND)
            return Napi::Value::From(env, false);
        CheckMdbxResult(rc);

        return Napi::Value::From(env, true);
    });
}

Napi::Value CppDbi::Has(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);

    ExtractBuffer(info[0], _keyBuffer);

    return wrapException(env, [&] () {
        MDBX_val key = CreateMdbxVal(_keyBuffer);
        MDBX_val value;

        const int rc = mdbx_get(_dbEnvPtr->GetTransaction(), _dbDbi, &key, &value);
        if (rc == MDBX_NOTFOUND)
            return Napi::Value::From(env, false);
        CheckMdbxResult(rc);

        return Napi::Value::From(env, true);
    });
}

Napi::Value CppDbi::HasDup(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);

    ExtractBuffer(info[0], _keyBuffer);
    ExtractBuffer(info[1], _dupKeyBuffer);

    return wrapException(env, [&] () {
        MDBX_cursor *dbCur = NULL;
        MDBX_val key = CreateMdbxVal(_keyBuffer);
        MDBX_val dupKey = CreateMdbxVal(_dupKeyBuffer);

        int rc = MDBX_SUCCESS;
        try {
            rc = mdbx_cursor_open(_dbEnvPtr->GetTransaction(), _dbDbi, &dbCur);
            CheckMdbxResult(rc);

            rc = mdbx_cursor_get(dbCur, &key, &dupKey, MDBX_GET_BOTH_RANGE);
            if (rc == MDBX_NOTFOUND)
                return Napi::Value::From(env, false);
            CheckMdbxResult(rc);

            mdbx_cursor_close(dbCur);
            return Napi::Value::From(env, true);

        } catch(...) {
            if (dbCur != NULL)
                mdbx_cursor_close(dbCur);
            throw;
        };

        return Napi::Value::From(env, false);
    });
}

Napi::Value CppDbi::FirstKey(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);

    return wrapException(env, [&] () {
        MDBX_cursor *dbCur = NULL;
        MDBX_val key;

        int rc = MDBX_SUCCESS;
        try {
            rc = mdbx_cursor_open(_dbEnvPtr->GetTransaction(), _dbDbi, &dbCur);
            CheckMdbxResult(rc);

            rc = mdbx_cursor_get(dbCur, &key, NULL, MDBX_FIRST);
            if (rc == MDBX_NOTFOUND)
                return env.Undefined();
            CheckMdbxResult(rc);

            Napi::Buffer<char> result = Napi::Buffer<char>::Copy(env, (const char *)key.iov_base, key.iov_len);

            mdbx_cursor_close(dbCur);

            return _outKey(result);
        } catch(...) {
            if (dbCur != NULL)
                mdbx_cursor_close(dbCur);
            throw;
        };

        return env.Undefined();
    });
}

Napi::Value CppDbi::LastKey(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);

    return wrapException(env, [&] () {
        MDBX_cursor *dbCur = NULL;
        MDBX_val key;

        int rc = MDBX_SUCCESS;
        try {
            rc = mdbx_cursor_open(_dbEnvPtr->GetTransaction(), _dbDbi, &dbCur);
            CheckMdbxResult(rc);

            rc = mdbx_cursor_get(dbCur, &key, NULL, MDBX_LAST);
            if (rc == MDBX_NOTFOUND)
                return env.Undefined();
            CheckMdbxResult(rc);

            Napi::Buffer<char> result = Napi::Buffer<char>::Copy(env, (const char *)key.iov_base, key.iov_len);

            mdbx_cursor_close(dbCur);

            return _outKey(result);
        } catch(...) {
            if (dbCur != NULL)
                mdbx_cursor_close(dbCur);
            throw;
        };

        return env.Undefined();
    });
}

Napi::Value CppDbi::NextKey(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);
    
    if (info[0].IsNull() || info[0].IsUndefined())
        return env.Undefined();

    ExtractBuffer(info[0], _keyBuffer);
    MDBX_val inKey = CreateMdbxVal(_keyBuffer);

    return wrapException(env, [&] () {
        MDBX_cursor *dbCur = NULL;
        MDBX_val key = inKey;

        int rc = MDBX_SUCCESS;
        try {
            rc = mdbx_cursor_open(_dbEnvPtr->GetTransaction(), _dbDbi, &dbCur);
            CheckMdbxResult(rc);

            rc = mdbx_cursor_get(dbCur, &key, NULL, MDBX_SET_RANGE);
            if (rc == MDBX_NOTFOUND)
                return env.Undefined();
            CheckMdbxResult(rc);

            const int cmpResult = mdbx_cmp(_dbEnvPtr->GetTransaction(), _dbDbi, &inKey, &key);
            if (cmpResult == 0) {
                rc = mdbx_cursor_get(dbCur, &key, NULL, MDBX_NEXT);
                if (rc == MDBX_NOTFOUND)
                    return env.Undefined();
                CheckMdbxResult(rc);
            };

            Napi::Buffer<char> result = Napi::Buffer<char>::Copy(env, (const char *)key.iov_base, key.iov_len);

            mdbx_cursor_close(dbCur);

            return _outKey(result);
        } catch(...) {
            if (dbCur != NULL)
                mdbx_cursor_close(dbCur);
            throw;
        };

        return env.Undefined();
    });
}

Napi::Value CppDbi::PrevKey(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    _check(env);
    
    if (info[0].IsNull() || info[0].IsUndefined())
        return env.Undefined();

    ExtractBuffer(info[0], _keyBuffer);
    MDBX_val inKey = CreateMdbxVal(_keyBuffer);

    return wrapException(env, [&] () {
        MDBX_cursor *dbCur = NULL;
        MDBX_val key = inKey;

        int rc = MDBX_SUCCESS;
        try {
            rc = mdbx_cursor_open(_dbEnvPtr->GetTransaction(), _dbDbi, &dbCur);
            CheckMdbxResult(rc);

            rc = mdbx_cursor_get(dbCur, &key, NULL, MDBX_SET_RANGE);
            if (rc == MDBX_NOTFOUND) {
                rc = mdbx_cursor_get(dbCur, &key, NULL, MDBX_LAST);
                if (rc == MDBX_NOTFOUND)
                    return env.Undefined();
            };
            CheckMdbxResult(rc);

            const int cmpResult = mdbx_cmp(_dbEnvPtr->GetTransaction(), _dbDbi, &inKey, &key);
            if (cmpResult <= 0) {
                rc = mdbx_cursor_get(dbCur, &key, NULL, MDBX_PREV);
                if (rc == MDBX_NOTFOUND)
                    return env.Undefined();
                CheckMdbxResult(rc);
            };

            Napi::Buffer<char> result = Napi::Buffer<char>::Copy(env, (const char *)key.iov_base, key.iov_len);

            mdbx_cursor_close(dbCur);

            return _outKey(result);
        } catch(...) {
            if (dbCur != NULL)
                mdbx_cursor_close(dbCur);
            throw;
        };

        return env.Undefined();
    });
}

Napi::Value CppDbi::LowerBoundKey(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info[0].IsNull() || info[0].IsUndefined())
        return FirstKey(info);

    _check(env);

    ExtractBuffer(info[0], _keyBuffer);
    MDBX_val inKey = CreateMdbxVal(_keyBuffer);

    return wrapException(env, [&] () {
        MDBX_cursor *dbCur = NULL;
        MDBX_val key = inKey;

        int rc = MDBX_SUCCESS;
        try {
            rc = mdbx_cursor_open(_dbEnvPtr->GetTransaction(), _dbDbi, &dbCur);
            CheckMdbxResult(rc);

            rc = mdbx_cursor_get(dbCur, &key, NULL, MDBX_SET_RANGE);
            if (rc == MDBX_NOTFOUND)
                return env.Undefined();
            CheckMdbxResult(rc);

            Napi::Buffer<char> result = Napi::Buffer<char>::Copy(env, (const char *)key.iov_base, key.iov_len);

            mdbx_cursor_close(dbCur);
            return _outKey(result);
        } catch(...) {
            if (dbCur != NULL)
                mdbx_cursor_close(dbCur);
            throw;
        };

        return env.Undefined();
    });
}

void CppDbi::_check(Napi::Env &env) {
    if (!_dbEnvPtr || !_dbEnvPtr->IsOpened())
        throw Napi::Error::New(env, "Closed.");

}

Napi::Value CppDbi::_outKey(Napi::Buffer<char> &buffer) {
    if (_dbEnvPtr->IsStringKeyMode())
        return buffer.ToString();
    return buffer;
}

Napi::Value CppDbi::_outValue(Napi::Buffer<char> &buffer) {
    if (_dbEnvPtr->IsStringValueMode())
        return buffer.ToString();
    return buffer;
}
