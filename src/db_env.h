#pragma once

#include <memory>
#include <string>
#include <map>

#include "mdbx.h"
#include "utils.h"

#include "db_exception.h"

const intptr_t MB = 1048576;

class DbEnv;

typedef std::shared_ptr<DbEnv> DbEnvPtr;

struct DbEnvParameters {
    std::string dbPath;
    bool readOnly = false;
    intptr_t pageSize = -1;
};

class DbEnv {
public:
    DbEnv() {

    }

    inline void Open(const DbEnvParameters &parameters) {
        if (_env)
            throw DbException("Already opened.");
    
        int rc = MDBX_SUCCESS;
        unsigned flags = 0;
        MDBX_env *env = NULL;

        try {
            rc = mdbx_env_create(&env);
            CheckMdbxResult(rc);

            rc = mdbx_env_set_geometry(
                env,
                -1, // size_lower
                -1, // size_now
                256 * 1024 * MB, // size_upper
                4 * MB, // growth_step
                16 * MB, // shrink_threshold
                parameters.pageSize
            );
            CheckMdbxResult(rc);

            flags = MDBX_ACCEDE | MDBX_LIFORECLAIM | MDBX_NOMETASYNC | MDBX_SAFE_NOSYNC;
            if (parameters.readOnly)
                flags |= MDBX_RDONLY;
            rc = mdbx_env_open(env, parameters.dbPath.c_str(), flags, 0666);
            CheckMdbxResult(rc);

            _env = env;
            _readOnly = parameters.readOnly;
        } catch(...) {
            if (env)
                mdbx_env_close(env);
            throw;
        };
    }

    inline void Close() {
        if (_env) {
            mdbx_env_close(_env);
            _env = NULL;
            _readOnly = false;
        };
    }

    inline bool IsOpened() {
        return _env != NULL;
    }

    inline bool IsReadOnly() {
        return _readOnly;
    }

    inline MDBX_dbi OpenDbi(const std::string &name) {
        // TODO: надо проверить, что мы не в транзакции уже
        _checkOpened();
        auto it = _openedDbis.find(name);
        if (it != _openedDbis.end())
            return it->second;

        int rc = MDBX_SUCCESS;
        unsigned flags = 0;
        MDBX_dbi dbi = 0;
        MDBX_txn *txn = NULL;

        try {
            if (_readOnly)
                flags |= MDBX_RDONLY;
            rc = mdbx_txn_begin(_env, NULL, flags, &txn);
            CheckMdbxResult(rc);

            flags = MDBX_CREATE;
            if (_readOnly)
                flags = 0;
            rc = mdbx_dbi_open(txn, name.empty() ? NULL : name.c_str(), 0, &dbi);
            CheckMdbxResult(rc);

            rc = mdbx_txn_commit(txn);
            CheckMdbxResult(rc);
        } catch(...) {
            if (txn)
                mdbx_txn_abort(txn);
            throw;
        };

        _openedDbis.emplace(name, dbi);
        return dbi;
    }

    inline void _checkOpened() {
        if (_env == NULL)
            throw DbException("Closed.");
    }

    ~DbEnv() {
        Close();
    }

private:
    bool _readOnly = false;
    MDBX_env *_env = NULL;
    std::map<std::string, MDBX_dbi> _openedDbis;
};