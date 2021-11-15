const mainDbi = Symbol();

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

    getDbi(name, dupsort) {
        const fixedName = this._fixName(name);
        let dbi = this._dbis[fixedName];
        if (dbi && dbi.isStale())
            dbi = undefined;
        if (!dbi)
            dbi = this._dbis[fixedName] = this._cppMdbx.getDbi(name, dupsort);
        return dbi;
    }

    clearDbi(txnId, name, remove) {
        this._check(txnId);
        if (remove)
            delete this._dbis[this._fixName(name)];
        return this._cppMdbx.clearDbi(name, !!remove);
    }

    _check(txnId) {
        if (txnId != this._txnId)
            throw new Error('Stale transaction.');
    }

    _fixName(name) {
        if (name != null)
            return name;
        return mainDbi;
    }
}

exports = module.exports = TxnManager;