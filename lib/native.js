const os = require('os');

let nativePath;

if (os.type() == 'Windows_NT') {
    nativePath = '../build/RelWithDebInfo/node-mdbx-native';
} else {
    nativePath = '../build/Release/node-mdbx-native';
};

const { CppMdbx } = require(nativePath);

exports.CppMdbx = CppMdbx;