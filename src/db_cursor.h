#pragma once

#include <napi.h>
#include "mdbx.h"
#include "db_cursor.h"

class DbCursor {
    DbCursor(MDBX_txn *dbTxn, MDBX_dbi dbDbi, MDBX_cursor_op openOp) {
        
    }

    ~DbCursor() {

    }
}