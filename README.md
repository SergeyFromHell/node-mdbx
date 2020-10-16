# **node-mdbx**
This module contains [MDBX](https://github.com/erthink/libmdbx) bindings.

MDBX is a fork of well-known LMDB embeddable database. It contains various fixes and improvements.
See [MDBX](https://github.com/erthink/libmdbx) for details.

## Installation
Currently, only linux environment is supported.

```bash
npm install node-mdbx
```

## Usage
See [API](#api) for details.

```js
const MDBX = require('node-mdbx');
const db = new MDBX({
  // path to DB directory
  path: '/tmp/test_db',
  
  // maximum number of inner Dbis (default is 1)
  maxDbs: 5,
  
  // Read-only mode (default is false)
  readOnly: false,
  
  // Page size (default is 4096)
  pageSize: 65536,
  
  // Key mode. Possible values:
  // 'string' (default) - all returned keys will be auto-converted from buffer to string
  // 'buffer' - returned keys will be buffers
  keyMode: 'string',
});

// All database operations should be enclosed inside transaction with transact method.
// It accepts *sync* function as only parameter. Nested calls are possible
// (but commit/rollback mechanics will only work at top-level .transact call).

const result = db.transact(function action {
  const dbi = db.getDbi('main');
  
  // working with dbi
  
  dbi.put('key', 'value);

  // Buffer.isBuffer(value) == true && String(value) == 'value'
  const value = dbi.get('key');
  
  // true
  dbi.has('key');
  
  dbi.del('key');
  
  // false
  dbi.has('key');
  
  // undefined
  dbi.get('key')
  
  ...
  
  // iterate over all values:
  for (let key = dbi.first(); key != null; key = dbi.next(key)) {
    console.log([
      key,
      String(dbi.get(key)),
    ]);
  }
  
  // returned value will be passed as a result of .transact call:
  return 'someresult';
});

// There is also an async version of transact: asyncTransact.
// It accepts *async* function as only parameter. 
// asyncTransact call queues execution of passed action until no transaction is active.

// Warning 1: Remember that during action execution other transactions cannot be processed,
//   so action should be as fast as possible.

// Warning 2: Deadlock will be created with await of nested asyncTransact call. To overcome it:
//   1. Don't use such a nested calls, or
//   2. Don't wait (asynchronously) on nested asyncTransact calls within initial asyncTransact call stack.

async function dbExample() {
  await db.asyncTransact(async function action() {
    const dbi = db.getDbi('main');
    
    // do something with dbi
    // can use async calls
  });
}
```

## API

- [class `MDBX`](#class-mdbx)
- [class `DBI`](#class-dbi)

# class *MDBX*

- [new MDBX()](#new-mdbxoptions)
- [MDBX#transact()](#transactaction)
- [MDBX#asyncTransact()](#asynctransactaction)
- [MDBX#close()](#close)
- [MDBX#closed](#closed)
- [MDBX#getDbi()](#getdbiname)
- [MDBX#hasTransaction()](#hastransaction)
- [MDBX#clearDb()](#static-cleardbpath)

### new MDBX(*options*)
Creates or opens MDBX database. Accepts `options` object.
- `options.path` - database directory path (required)
- `options.readOnly` - if true, then opens in read-only mode (default: false)
- `options.maxDbs` - maximum number of inner Dbis (default: 1)
- `options.pageSize` - page size (default: 4096)
- `options.keyMode` - key mode:
  * 'string' - all key-returning methods convert keys from buffer to string (default)
  * 'buffer' - no key conversion

### .transact(*action*)
Executes *syncronous* action inside transaction. If transaction is already active, then uses it.
Rollbacks on error (only for top-level .transact call). Returns the returned value of action call.

### .asyncTransact(*action*)
Executes *async* action inside transaction. Queues execution if needed.
* Warning! Avoid nested .asyncTransact awaits as it could lead to a deadlock! *

### .close()
Closes the database.

### .closed
Has true value if database has been closed.

### .getDbi(*name*)
Opens and returns DBI of a given name (null or empty string - open main/default dbi).

### .hasTransaction()
Returns true if there is a transaction active.

### static clearDb(*path*)
Deletes whole database by it's directory path.
* Database should not be opened in any process! *

# class *DBI*
- [DBI#put()](#putkey-value)
- [DBI#get()](#getkey)
- [DBI#has()](#haskey)
- [DBI#del()](#delkey)
- [DBI#first()](#first)
- [DBI#last()](#last)
- [DBI#next()](#nextkey)
- [DBI#prev()](#prevkey)
- [DBI#lowerBound()](#lowerboundkey)

### .put(*key*, *value*)
Set value of a key. Key and value should be Buffer or string.

### .get(*key*)
Get value of a *key*. Returns Buffer with it's value if such a key exists. Returns undefined otherwise.

### .has(*key*)
Returns true if *key* exists. Returns false otherwise.

### .del(*key*)
Deletes *key*.

### .first()
Returns the smallest (lexicographically) key in Dbi. If there are no keys returns undefined.

### .last()
Returns the biggest (lexicographically) key in Dbi. If there are no keys returns undefined.

### .next(*key*)
Returns the smallest (lexicographically) key greater than the given input *key*.
It there are no such a keys, returns undefined.

### .prev(*key*)
Returns the largest (lexicographically) key less than the given input *key*.
It there are no such a keys, returns undefined.

### .lowerBound(*key*)
Returns the smallest (lexicographically) key greater or equal to the given input *key*.
It there are no such a keys, returns undefined.


