#pragma once

#include <memory>
#include <string>
#include <map>
#include <set>

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
    unsigned maxDbs = 0;
    bool stringKeyMode = true;
    bool stringValueMode = false;
};

class DbEnv {
public:
    void Open(const DbEnvParameters &parameters);
    void Close();
    bool IsOpened();
    bool IsReadOnly();

    MDBX_dbi OpenDbi(const std::string &name);

    void BeginTransaction();
    void CommitTransaction();
    void AbortTransaction();
    bool HasTransaction();
    MDBX_txn * GetTransaction();
    bool IsStale(const std::string &name, MDBX_dbi dbi);
    bool IsStringKeyMode();
    bool IsStringValueMode();

    ~DbEnv();

private:
    void _checkTransaction();
    void _checkNotTransaction();
    void _checkOpened();

    bool _readOnly = false;
    bool _stringKeyMode = true;
    bool _stringValueMode = false;
    MDBX_env *_env = NULL;
    MDBX_txn *_txn = NULL;
    std::map<std::string, MDBX_dbi> _openedDbis;
    std::set<std::string> _pendingTransactionDbis;
};