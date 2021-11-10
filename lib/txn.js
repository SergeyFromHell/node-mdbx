class Txn {
    constructor(txnManager) {
        this._txnManager = txnManager;
        this._txnId = txnManager.beginTransaction();
    }

    _finish() {
        this._txnManager = null;
        this._txnId = 0;
    }

    getDbi(name, dupsort) {
        return this._txnManager.getDbi(name, dupsort);
    }

    clearDbi(name, remove) {
        return this._txnManager.clearDbi(this._txnId, name, remove);
    }

    finished() {
        return this._txnManager == null;
    }

    commit() {
        try {
            this._txnManager.commitTransaction(this._txnId);
        } finally {
            this._finish();
        };
    }

    abort() {
        try {
            this._txnManager.abortTransaction(this._txnId);
        } finally {
            this._finish();
        };
    }
};

exports = module.exports = Txn;