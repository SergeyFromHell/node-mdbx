'use strict';
const { CppMdbx } = require('../build/Release/node-mdbx-native');
const wrapFunctions = require('./wrap_functions');

function MDBX(...args) {
    const instance = new CppMdbx(...args);
    wrapFunctions(instance, this);
}

module.exports = MDBX;