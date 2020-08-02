/*
  You need to add '#define NANOPULSE_DB_IMPLEMENTATION' before including this header in ONE source file.
  Like this:
      #define NANOPULSE_DB_IMPLEMENTATION
      #include "nanopulsedb.h"
 
  To force compilation as C99 add:
      #define DISABLE_CPP // This will also disable compile-time unit tests
 
  LICENSE
      See end of file.
 
  DONATE
      This project cannot continue without YOU! Please support! It is MUCH appreciated!
        BTC:  1H1RrCrEgUXDFibpaJciLjS9r7upQs6XPc
        BCH:  qzgfgd6zen70mfzasjtc4rx9m7fhz65zyg0n6v3sdh
        BSV:  15dtAGzzMf6yWF82aYuGKZYMCyP5HoWVLP
        ETH:  0x32a42d02eB021914FE8928d4A60332970F96f2cd
        DCR:  DsWY2Z1NThKqumM6x9oiyM3f2RkW28ruoyA
        LTC:
        DASH: XqMBmnxrgJWsvF7Hu3uBQ53TpcKLEsxsEi
      Thank you in advance!
      
*/

#ifndef NANOPULSE_DB_H
#define NANOPULSE_DB_H

#if defined(__cplusplus) && !defined(DISABLE_CPP)
  #define COMP_AS_CPP
#endif

#if defined(COMP_AS_CPP)
  #define COMPTIME constexpr
  #define CTOR4(w,x,y,z) constexpr w,x,y,z
#else
  #define COMPTIME static const
  #define nullptr NULL
  enum {false=0,true=1};
  typedef int bool;
  #define CTOR4(w,x,y,z)
#endif


struct nplse_bitvec;

typedef unsigned (*pnplse_bitvec_alloc)(struct nplse_bitvec *bitvec, int amount);

typedef struct nplse_bitvec
{
    unsigned *bitvec;
    int size;
    pnplse_bitvec_alloc nplse_bitvec_alloc;
} nplse_bitvec;


typedef struct nplse_skipnode
{
    CTOR4(nplse_skipnode() : nodeid(0), filepos(0), prev(0), next(0) {})
    unsigned nodeid;
    int filepos;
    int prev, next; // offsets
} nplse_skipnode;

/*
 1. ''priority' list
 
 top list: must have 80%+
 next:               16%
 next                 3%
 next                 1%
 
 
 2.hash
 2. dyn array
 3. bitvec
 4. bloom
 */

struct nanopulseDB
{
    // File:
    unsigned char *mappedfile;
    nplse_bitvec occupied;
    int szMappedfile;
    int chunkSize;
    
    // List:
    nplse_skipnode *nodeVec;
    int rootA=0, rootB=0, rootC=0, rootD=0;
    int nActiveNodes = 0;
    int nAllocatedNodes = 32;
    int designatedNewNode = 0;
    
    // Hashing:
    unsigned seed = 0;
};




// Public interface
//static nplse *nplse_open(nplse_internal_test *test);
//static void nplse_close(nplse *instance);




#ifdef NANOPULSE_DB_IMPLEMENTATION



#endif // NANOPULSE_DB_IMPLEMENTATION


#endif // NANOPULSE_DB_H

/*
 ------------------------------------------------------------------------------
 Copyright (c) 2020 Stephen van Helsing
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
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
