#!/usr/bin/env python

config_file = "src/includedb_config.h"

files = [
("src/includedb_core.h"    , [                                                   ]),
("src/includedb_bloom.c"   , ["src/includedb_config.h","src/includedb_bloom.h"]),
("src/includedb_bitvec.c"  , ["src/includedb_config.h","src/includedb_bitvec.h"  ]),
("src/includedb_skiplist.c", ["src/includedb_config.h","src/includedb_skiplist.h"]),
("src/includedb_fileops.c" , ["src/includedb_config.h","src/includedb_fileops.h" ]),
("src/includedb_core.c"    , ["src/includedb_config.h","src/includedb_core.h"    ])   
]

# for testing only:
mix = [
("g", ["d","f","e"]),
("a", []),
("f", ["b","c"]),
("b", []),
("c", []),
("d", ["e","c"]),
("e", ["a","b"])
]

def filter(i,sources):
    new = []
    for elem in sources:
        if elem[0]==i:
            new.append(elem)
    return new

def extr(tup, sources):
    new = []
    for i in tup[1]:
        new += filter(i, sources)
    return new

def arrange(sources):
    new = []
    for elem in reversed(sources):
        new += extr(elem, sources)
    return new
    
sorted = arrange(files) + files
unique = []
[unique.append(elem) for elem in sorted if elem not in unique]
for c in unique:
    print(c[0]+" - "+" ".join(c[1]))

merge_result = """/*
  You need to add '#define INCLUDEDB_IMPLEMENTATION' before including this header in ONE source file.
  Like this:
      #define INCLUDEDB_IMPLEMENTATION
      #include \"includedb.h\"
 
  To disable compile-time unit tests:
      #define DISABLE_TESTS
 
  AUTHOR
      Professor Peanut
 
  LICENSE
      MIT - See end of file.
 
  HISTORY
      While in alpha, cross version compatibility is not guaranteed. Do not replace
      this file if you need to keep your database readable.

      0.1.1  Splitting into multiple files, use generator.py to create single-header!
      0.1.0  Initial release
 
  PEANUTS
      BTC:  bc1qpv63qlpec3x3lh2cejmr5audh2c6j2ar3ptvy235hld3f2wwzr5sg4qr5n
      BCH:  qzfy6xcw93s8605rywcwug3vpf5kmy5ywgw0lw5lj0
      ETH:  0x32a42d02eB021914FE8928d4A60332970F96f2cd
      LTC:  LPM7ueXta6kFwCnBKd5viJDX2CN8eLsg3b
      XMR:  47NF2hjeMXMMCHu6XNyMrWeJwkndaTNvGAKAQuy6v9wvNTHViVwi3BGTr8wy9U4aoNbDcLMEf7dVjNGvQacttGc3CjEJgP8
      Let's cook this stonesoup together!!!
      
*/\n\n"""

license = """/*
 ------------------------------------------------------------------------------
 Copyright (c) 2023 Professor Peanut
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 This copyright notice and permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 ------------------------------------------------------------------------------
*/
"""

merge_result += "#ifndef INCLUDEDB_DB_H\n#define INCLUDEDB_DB_H\n\n"
merge_result += open(config_file, "r").read()
merge_result += "\n\n#ifdef INCLUDEDB_IMPLEMENTATION\n\n\n"
for filename in unique:
    if filename[0] != config_file:
        f = open(filename[0], "r")
        lines = f.readlines()
        for code_line in lines:
            if code_line.startswith("#include \"") == False:
                merge_result += code_line
        merge_result += "\n\n"

merge_result += "#endif // INCLUDEDB_IMPLEMENTATION\n\n\n#endif // INCLUDEDB_DB_H\n"
merge_result+= license

open("./includedb.h", "w+").write(merge_result + "\n")

print("success")


