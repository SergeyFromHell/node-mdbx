#include "db_env.h"

void DbEnv::Open(const DbEnvParameters &parameters) {
    if (_env)
        throw DbException("Already opened.");

    int rc = MDBX_SUCCESS;
    MDBX_env *env = NULL;

    try {
        rc = mdbx_env_create(&env);
        CheckMdbxResult(rc);

        if (parameters.maxDbs > 0) {
            rc = mdbx_env_set_maxdbs(env, (MDBX_dbi) parameters.maxDbs);
            CheckMdbxResult(rc);
        };

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

        MDBX_env_flags_t envFlags = MDBX_ACCEDE | MDBX_LIFORECLAIM | (MDBX_env_flags_t) parameters.syncMode;
        if (parameters.readOnly)
            envFlags |= MDBX_RDONLY;
        rc = mdbx_env_open(env, parameters.dbPath.c_str(), envFlags, 0666);
        CheckMdbxResult(rc);

        _env = env;
        _readOnly = parameters.readOnly;
        _stringKeyMode = parameters.stringKeyMode;
        _stringValueMode = parameters.stringValueMode;
    } catch(...) {
        if (env)
            mdbx_env_close(env);
        throw;
    };
}

void DbEnv::Close() {
    if (_env) {
        mdbx_env_close(_env);
        _env = NULL;
        _readOnly = false;
        _txn = NULL;
        _pendingTransactionDbis.clear();
        _openedDbis.clear();
    };
}

bool DbEnv::IsOpened() {
    return _env != NULL;
}

bool DbEnv::IsReadOnly() {
    return _readOnly;
}

MDBX_dbi DbEnv::OpenDbi(const std::string &name, bool dupsort) {
    _checkOpened();

    auto it = _openedDbis.find(name);
    if (it != _openedDbis.end())
        return it->second;

    int rc = MDBX_SUCCESS;
    MDBX_dbi dbi = 0;
    MDBX_txn *txn = NULL;

    try {
        if (_txn == NULL) {
            MDBX_txn_flags_t txnFlags = MDBX_TXN_READWRITE;
            if (_readOnly)
                txnFlags |= MDBX_TXN_RDONLY;
            rc = mdbx_txn_begin(_env, NULL, txnFlags, &txn);
            CheckMdbxResult(rc);
        };

        MDBX_db_flags_t dbFlags = MDBX_DB_DEFAULTS;
        if (!_readOnly)
            dbFlags = MDBX_CREATE;
        if (dupsort)
            dbFlags |= MDBX_DUPSORT;
        rc = mdbx_dbi_open((_txn != NULL) ? _txn : txn, name.empty() ? NULL : name.c_str(), dbFlags, &dbi);
        CheckMdbxResult(rc);

        if (_txn == NULL) {
            rc = mdbx_txn_commit(txn);
            CheckMdbxResult(rc);
        };
    } catch(...) {
        if (txn)
            mdbx_txn_abort(txn);
        throw;
    };

    _openedDbis.emplace(name, dbi);
    if (_txn != NULL)
        _pendingTransactionDbis.insert(name);
    return dbi;
}

void DbEnv::ClearDbi(const std::string &name, bool remove, bool dupsort) {
    _checkTransaction();
    MDBX_dbi dbi = OpenDbi(name, dupsort);
    if (remove)
        _openedDbis.erase(name);
    const int rc = mdbx_drop(_txn, dbi, remove);
    CheckMdbxResult(rc);
}

void DbEnv::_checkOpened() {
    if (_env == NULL)
        throw DbException("Closed.");
}

void DbEnv::BeginTransaction() {
    _checkNotTransaction();

    MDBX_txn_flags_t txnFlags = MDBX_TXN_READWRITE;
    if (_readOnly)
        txnFlags |= MDBX_TXN_RDONLY;
    const int rc = mdbx_txn_begin(_env, NULL, txnFlags, &_txn);
    CheckMdbxResult(rc);
}

void DbEnv::CommitTransaction() {
    _checkTransaction();

    const int rc = mdbx_txn_commit(_txn);
    _txn = NULL;

    _pendingTransactionDbis.clear();

    CheckMdbxResult(rc);
}

void DbEnv::AbortTransaction() {
    _checkTransaction();

    const int rc = mdbx_txn_abort(_txn);
    _txn = NULL;

    for (const auto &name : _pendingTransactionDbis)
        _openedDbis.erase(name);

    _pendingTransactionDbis.clear();

    CheckMdbxResult(rc);
}

bool DbEnv::HasTransaction() {
    return _txn != NULL;
}

MDBX_txn * DbEnv::GetTransaction() {
    _checkTransaction();
    return _txn;
}

bool DbEnv::IsStale(const std::string &name, MDBX_dbi dbi) {
    auto it = _openedDbis.find(name);
    return it == _openedDbis.end() || it->second != dbi;
}

bool DbEnv::IsStringKeyMode() {
    return _stringKeyMode;
}

bool DbEnv::IsStringValueMode() {
    return _stringValueMode;
}

void DbEnv::_checkTransaction() {
    if (_txn == NULL)
        throw DbException("No transaction started.");
}

void DbEnv::_checkNotTransaction() {
    if (_txn != NULL)
        throw DbException("Multiple parallel transactions.");
}

DbEnv::~DbEnv() {
    Close();
}
