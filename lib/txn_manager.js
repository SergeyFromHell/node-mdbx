class TxnManager {
    constructor(cppMdbx) {
        this._cppMdbx = cppMdbx;
        this._dbis = Object.create(null);
        this._txnCounter = 0;
        this._txnId = 1;
    }

    beginTransaction() {
        if (this._txnCounter == 0)
            this._cppMdbx.beginTransaction();
        this._txnCounter++;
        return this._txnId;
    }

    commitTransaction(txnId) {
        this._check(txnId);
        this._txnCounter--;
        if (this._txnCounter == 0) {
            this._txnId++;
            this._cppMdbx.commitTransaction();
        };
    }

    abortTransaction(txnId) {
        this._check(txnId);
        this._txnCounter--;
        if (this._txnCounter == 0) {
            this._txnId++;
            this._cppMdbx.abortTransaction();
        };
    }

    getDbi(name) {
        let dbi = this._dbis[name];
        if (dbi && dbi.isStale())
            dbi = undefined;
        if (!dbi)
            dbi = this._dbis[name] = this._cppMdbx.getDbi(name);
        return dbi;
    }

    _check(txnId) {
        if (txnId != this._txnId)
            throw new Error('Stale transaction.');
    }
}

exports = module.exports = TxnManager;