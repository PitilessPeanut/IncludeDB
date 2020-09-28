# IncludeDB

An [stb](https://github.com/nothings/stb/)-style *#include*-only single-header k/v store implemented in C99 with an emphasis on easy-of-use.
Meant to be used in environments with limited resources or in situations where a more complex database library would be an overkill such as small games.

## THIS PROJECT NEEDS YOUR SUPPORT!

If you like this project or find it useful in any way, please help out - even just a little bit! It helps immensely!
Donate here:

<img src="images/btc.png" width="150px" alt="1H1RrCrEgUXDFibpaJciLjS9r7upQs6XPc"><img src="images/bch.png" width="150px" alt="qzgfgd6zen70mfzasjtc4rx9m7fhz65zyg0n6v3sdh"><img src="images/bsv.png" width="150px" alt="15dtAGzzMf6yWF82aYuGKZYMCyP5HoWVLP"><img src="images/eth.png" width="150px" alt="0x32a42d02eB021914FE8928d4A60332970F96f2cd">
DCR:  DsWY2Z1NThKqumM6x9oiyM3f2RkW28ruoyA
LTC:  LWZ5HCcpModc1XcFpjEzz25J58eeQ8fJ7F
DASH: XqMBmnxrgJWsvF7Hu3uBQ53TpcKLEsxsEi

**Attention!! This project is *early* alpha and many things have yet to be implemented:**
- memory mapping
- Support for concurrent read
- Support for thread-safe write
- Proper Windows support
- *delete* a key/value pair
- recover keys
- benches
- maybe more stuff, I'm open for suggestions...
- **A better logo is needed!** If you would like to volunteer to make one that would be super appreciated!!!

### Table of Contents
- [Building](#building)
  - [Demo](#demo)
  - [Compile-time testing](#compile-time-tests)
- [Using](#using)
  - [Visualization using Javascript](#javascript-visualization)
  -[Known issues](#known-issues)
- [Contributing](#contributing)
- [Similar projects](#similar-projects)

## Building
A build process is not required, simply drop *includedb.h* into your project and use.
#### Demo
Building the example program can be done with just ``cc -o demo demo_includedb.c`` When run, a _"hello.dat"_ file will be created.
#### Compile-time tests
If compiled as C++ a number of unit-tests will be executed during compilation. To disable this behaviour ```#define DISABLE_TESTS``` before *#including* this file.

## Using
_**Attention** This code is not yet production-ready! Proceed accordingly!_

Configuration:
More options, such as setting a different malloc()/free() are available. Take a look inside *nanopulsedb.h*
```
// Chunk size is 256 by default. This is the MINIMUM size
// a key/value pair is going to take up. To set a custom value:
#define NANOPULSE_CHUNK_SIZE 1024 // Optional

// After that add
#define INCLUDEDB_IMPLEMENTATION // Required!
// And finally
#include "includedb.h"

```
Usage:
```
// Create new or open existing. NULL is returned 
// in case the file couldn't be opened or created:
nanopulseDB *mydb = nplse_open("hello.dat");

// Make sure to call close() after done using:
nplse_close(mydb);
```
The database can also be operated entirely during compile time, providing you with a compile-time key/value store.

#### Javascript visualization
*algo-visualize.js* is provided and can be run on algorithm-visualizer.org to show the mechanism of the underlying algorithm. 

#### Known issues
During alpha there are still some issues we are working on. Documented issues:
'nplse_put()' may call 'nplse__write()' with uninitialized data 

## Contributing
I hope the code is somewhat readable and not too much of a mess. Either way contributions are very welcome, feel free to msg me or issue a request!

## Similar projects
IncludeDB is a "storage engine" more than a database with limited features designed for a niche purpose. If you need a more established library we encourage you to check out these:

[RocksDB](https://github.com/facebook/rocksdb) High performance, log based k/v 

[LMDB](https://github.com/LMDB/lmdb) Very popular                                              

[NuDB](https://github.com/CPPAlliance/NuDB) Header only k/v

[SQLite](https://github.com/sqlite/sqlite) Battle tested. Popular in browsers/etc. 

[Unqlite](https://github.com/symisc/unqlite)          

[Sparkey](https://github.com/spotify/sparkey)
                                                                                                  
