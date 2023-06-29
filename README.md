# IncludeDB

An [stb](https://github.com/nothings/stb/)-style *#include*-only single-header k/v store implemented in 
C99 with an emphasis on easy-of-use. <em>Small file database (!)</em> to be used in environments with 
limited resources or in situations where a more complex database library would be an overkill such as 
small games.

## THIS PROJECT NEEDS YOUR SUPPORT!

If you like this project or find it useful in any way, please help out - even just a little bit! It helps immensely!
Donate here:

####
<img src="images/btc.png" width="150px" alt="bc1qpv63qlpec3x3lh2cejmr5audh2c6j2ar3ptvy235hld3f2wwzr5sg4qr5n">
bc1qpv63qlpec3x3lh2cejmr5audh2c6j2ar3ptvy235hld3f2wwzr5sg4qr5n
####
<img src="images/bch.png" width="150px" alt="bitcoincash:qzfy6xcw93s8605rywcwug3vpf5kmy5ywgw0lw5lj0.PNG">
bitcoincash-qzfy6xcw93s8605rywcwug3vpf5kmy5ywgw0lw5lj0.PNG
####
<img src="images/ltc.png" width="150px" alt="LPM7ueXta6kFwCnBKd5viJDX2CN8eLsg3b">
LPM7ueXta6kFwCnBKd5viJDX2CN8eLsg3b
####
<img src="images/xmr.png" width="150px" alt="47NF2hjeMXMMCHu6XNyMrWeJwkndaTNvGAKAQuy6v9wvNTHViVwi3BGTr8wy9U4aoNbDcLMEf7dVjNGvQacttGc3CjEJgP8">
47NF2hjeMXMMCHu6XNyMrWeJwkndaTNvGAKAQuy6v9wvNTHViVwi3BGTr8wy9U4aoNbDcLMEf7dVjNGvQacttGc3CjEJgP8
####
<img src="images/eth.png" width="150px" alt="ethereum:0x32a42d02eB021914FE8928d4A60332970F96f2cd">
ethereum:0x32a42d02eB021914FE8928d4A60332970F96f2cd


**Attention!! This project is still *alpha* and many things have yet to be implemented:**
- memory mapping
- Support for concurrent read
- Support for thread-safe write
- Proper Windows support
- *delete* a key/value pair
- recover keys
- benches
- maybe more stuff, I'm open for suggestions...
- **A logo is needed!** If you would like to volunteer to make one that would be super appreciated!!!

### Table of Contents
- [Building](#building)
  - [Demo](#demo)
  - [Compile-time testing](#compile-time-tests)
- [Using](#using)
  - [Visualization using Javascript](#javascript-visualization)
  - [Known issues](#known-issues)
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
More options, such as setting a different malloc()/free() are available. Take a look inside *includedb.h*
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
                                                                                                  
