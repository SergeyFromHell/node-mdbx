#!/usr/bin/env bash

DIR=$(cd "$(dirname "$0")"; pwd)
set -ex
cd $DIR/..

if [ ! -d "libmdbx" ] ; then
git clone git@github.com:erthink/libmdbx.git
cd libmdbx
else
cd libmdbx
git pull
fi

make dist

cp dist/mdbx.c deps/mdbx/
cp dist/mdbx.h deps/mdbx/
