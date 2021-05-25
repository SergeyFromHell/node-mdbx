'use strict';
const fs = require('fs');
const path = require('path');
const Txn = require('./txn');
const TxnManager = require('./txn_manager');
const createDeferred = require('./create_deferred');
const { CppMdbx } = require('./native');

class MDBX {
    constructor(options) {
        this._cppMdbx = new CppMdbx(options);
        this._txnManager = new TxnManager(this._cppMdbx);
        this._queue = [];
        this._closed = false;
        this._processingTransactionsQueue = false;
        this._processTransactionsQueue = this._processTransactionsQueue.bind(this);
    }

    close() {
        this._closed = true;
        this._cppMdbx.close();
    }

    get closed() {
        return this._closed;
    }

    _getTransaction() {
        return new Txn(this._txnManager);
    }

    transact(action) {
        if (typeof(action) != 'function')
            throw new Error('Action is not a function.');
        this._checkClosed();
        const transaction = this._getTransaction();
        let abort = false;
        try {
            return action(transaction);
        } catch(error) {
            abort = true;
            throw error;
        } finally {
            if (!transaction.finished()) {
                this._checkClosed();
                if (abort) {
                    try {
                        transaction.abort();
                    } catch(error) {};
                } else {
                    transaction.commit();
                };
            };
        };
    }

    async asyncTransact(action) {
        if (typeof(action) != 'function')
            throw new Error('Action is not a function.');
        const deferred = createDeferred();
        deferred.action = action;
        this._queue.push(deferred);

        if (!this._processingTransactionsQueue) {
            this._processingTransactionsQueue = true;
            setImmediate(this._processTransactionsQueue)
        };

        return deferred.promise;
    }

    hasTransaction() {
        return this._cppMdbx.hasTransaction();
    }

    async _doTransactAsync(action) {
        this._checkClosed();
        const transaction = this._getTransaction();
        let abort = false;
        try {
            return await action(transaction);
        } catch(error) {
            abort = true;
            throw error;
        } finally {
            if (!transaction.finished()) {
                this._checkClosed();
                if (abort) {
                    try {
                        transaction.abort();
                    } catch(error) {};
                } else {
                    transaction.commit();
                };
            };
        };
    }

    async _processTransactionsQueue() {
        while (this._queue.length) {
            const deferred = this._queue.shift();
            try {
                const result = await this._doTransactAsync(deferred.action);
                deferred.resolve(result);
            } catch(error) {
                deferred.reject(error);
            };
        };
        this._processingTransactionsQueue = false;
    }

    _checkClosed() {
        if (this._closed)
            throw new Error('Database has been closed.');
    }

    static clearDb(dbPath) {
        try {
            fs.unlinkSync(path.join(dbPath, 'mdbx.dat'));
        } catch(error) {
            if (error.code != 'ENOENT')
                throw error;
        };
    }
}

module.exports = MDBX;