'use strict';
const fs = require('fs');
const path = require('path');
const { CppMdbx } = require('../build/Release/node-mdbx-native');
const createDeferred = require('./create_deferred');

class MDBX {
    constructor(options) {
        this._instance = new CppMdbx(options);
        this._queue = [];
        this._processingTransactionsQueue = false;
        this._closed = false;
    }

    close() {
        this._closed = true;
        this._instance.close();
    }

    get closed() {
        return this._closed;
    }

    getDbi(name = null) {
        this._checkClosed();
        return this._instance.getDbi(name);
    }

    transact(action) {
        if (typeof(action) != 'function')
            throw new Error('Action is not a function.');
        this._checkClosed();
        if (this._instance.hasTransaction())
            return action(this);
        this._instance.beginTransaction();
        let abort = false;
        try {
            return action(this);
        } catch(error) {
            abort = true;
            throw error;
        } finally {
            this._checkClosed();
            if (abort) {
                try {
                    this._instance.abortTransaction();
                } catch(error) {};
            } else {
                this._instance.commitTransaction();
            };
        };
    }

    async asyncTransact(action) {
        if (typeof(action) != 'function')
            throw new Error('Action is not a function.');
        const deferred = createDeferred();
        deferred.action = action;
        this._queue.push(deferred);

        if (!this._processingTransactionsQueue)
            this._processTransactionsQueue();

        return deferred.promise;
    }

    hasTransaction() {
        return this._instance.hasTransaction();
    }

    async _doTransactAsync(action) {
        this._checkClosed();
        this._instance.beginTransaction();
        let abort = false;
        try {
            return await action(this);
        } catch(error) {
            abort = true;
            throw error;
        } finally {
            this._checkClosed();
            if (abort) {
                try {
                    this._instance.abortTransaction();
                } catch(error) {};
            } else {
                this._instance.commitTransaction();
            };
        };
    }

    async _processTransactionsQueue() {
        this._processingTransactionsQueue = true;
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