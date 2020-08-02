# NanopulseDB

An [stb](https://github.com/nothings/stb/)-style *#include*-only single-header k/v store implemented in C99 with an emphasis on easy-of-use.
Meant to be used in environments with limited resources or in situations where a more complex database library would be an overkill such as small games.

## THIS PROJECT NEEDS YOUR SUPPORT!

If you like this project or find it useful in any way, please please PLEASE consider helping out - even just a little bit! It helps immensely!
Donate here:

<img src="images/btc.png" width="50%" alt="1H1RrCrEgUXDFibpaJciLjS9r7upQs6XPc">
<img src="images/bch.png" width="50%" alt="qzgfgd6zen70mfzasjtc4rx9m7fhz65zyg0n6v3sdh">

**Attention!! This project is *early* alpha and many things have yet to be implemented:**
- *mmap()*ed file
- Support for concurrent read
- Support for thread-safe write
- Windows support has not been tested
- Write ahead logging
- Bloom filters
- *delete* a key/value pair
- large file stress tests
- maybe more stuff, such as a standalone version. I'm open for suggestions...
- **A logo is needed!** If you would like to volunteer to make one that would be super appreciated!!!


## Building
A build process is not required, simply drop *nanopulsedb.h* into your project and use.
#### Compile-time tests
If compiled as C++ a number of unit-tests will be executed during compilation. To disable this behaviour ```#define DISABLE_CPP``` before *#including* this file.

## Using
_**Attention** This database is not yet production-ready! Proceed accordingly!_



## Similar projects
Nanopulse is a tiny database engine with limited features designed for a niche purpose. If you need a more established library we encourage you to check out these:
[RocksDB](https://github.com/facebook/rocksdb) High performance, log based k/v 

[LMDB](https://github.com/LMDB/lmdb) Very popular                                              

[NuDB](https://github.com/CPPAlliance/NuDB) Header only k/v

[SQLite](https://github.com/sqlite/sqlite) Battle tested. Popular in browsers/etc. Not a key/value store                     

[Unqlite](https://github.com/symisc/unqlite)                                                                        
                                                                                                  
