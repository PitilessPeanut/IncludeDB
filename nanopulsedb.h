/*
  You need to add '#define NANOPULSE_DB_IMPLEMENTATION' before including this header in ONE source file.
  Like this:
      #define NANOPULSE_DB_IMPLEMENTATION
      #include "nanopulsedb.h"
 
  To force compilation as C99 add:
      #define DISABLE_CPP // This will also disable compile-time unit tests
 
  LICENSE
      See end of file.
 
  HISTORY
      0.1.0  Initial release
 
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
  #define CTOR3(x,y,z) constexpr x,y,z
#else
  #define COMPTIME static const
  #define constexpr
  #define nullptr NULL
  enum {false=0,true=1};
  typedef int bool;
  #define CTOR3(x,y,z)
#endif


struct nplse__bitvec;

typedef unsigned (*pnplse__bitvecAlloc)(struct nplse__bitvec *bitvec, int amount);

typedef struct nplse__bitvec
{
    CTOR3(nplse__bitvec() : bitvec(nullptr), size(0), nplse__bitvecAlloc(nullptr) {} )
    unsigned *bitvec;
    int size;
    pnplse__bitvecAlloc nplse__bitvecAlloc;
} nplse__bitvec;


typedef struct nplse__skipnode
{
    CTOR3(nplse__skipnode() : nodeid(0), filepos(0), next(0) {})
    unsigned nodeid;
    int filepos;
    int next; // offset
} nplse__skipnode;

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

typedef struct nanopulseDB
{
    // File:
    unsigned char *mappedfile;
    nplse__bitvec occupied;
    int szMappedfile;
    int chunkSize;
    
    // List:
    nplse__skipnode *nodeVec;
    int rootA=0, rootB=0, rootC=0, rootD=0;
    int nKeys = 0;
    int nAllocatedSlots = 32;
    //int addressNewNode = 0;
    
    // Hashing:
    unsigned seed = 0;
} nanopulseDB;




// Public interface
//static nplse *nplse_open(*test);
//static void nplse_close(nplse *instance);




#ifdef NANOPULSE_DB_IMPLEMENTATION

static constexpr unsigned nplse__xx32(const unsigned char *input, int len, unsigned seed)
{
    // https://github.com/Cyan4973/xxHash
    COMPTIME unsigned XXH_PRIME32_1 = 0x9E3779B1u;
    COMPTIME unsigned XXH_PRIME32_2 = 0x85EBCA77u;
    COMPTIME unsigned XXH_PRIME32_3 = 0xC2B2AE3Du;
    COMPTIME unsigned XXH_PRIME32_4 = 0x27D4EB2Fu;
    COMPTIME unsigned XXH_PRIME32_5 = 0x165667B1u;
    const unsigned char *bEnd = input+len;
    unsigned h=0, bytes=0;
    #define XXH_rotl32(x,r) (((x) << (r)) | ((x) >> (32 - (r))))
    if (len>=16)
    {
        const unsigned char *const limit = bEnd - 15;
        unsigned v1 = seed + XXH_PRIME32_1 + XXH_PRIME32_2;
        unsigned v2 = seed + XXH_PRIME32_2;
        unsigned v3 = seed + 0;
        unsigned v4 = seed - XXH_PRIME32_1;
        do
        {
            bytes = (input[0]+0u) | ((input[1]+0u)<< 8) | ((input[2]+0u)<<16) | ((input[3]+0u)<<24);
            v1 += bytes * XXH_PRIME32_2;
            v1  = XXH_rotl32(v1, 13);
            v1 *= XXH_PRIME32_1;
            input += 4;
            bytes = (input[0]+0u) | ((input[1]+0u)<< 8) | ((input[2]+0u)<<16) | ((input[3]+0u)<<24);
            v2 += bytes * XXH_PRIME32_2;
            v2  = XXH_rotl32(v2, 13);
            v2 *= XXH_PRIME32_1;
            input += 4;
            bytes = (input[0]+0u) | ((input[1]+0u)<< 8) | ((input[2]+0u)<<16) | ((input[3]+0u)<<24);
            v3 += bytes * XXH_PRIME32_2;
            v3  = XXH_rotl32(v3, 13);
            v3 *= XXH_PRIME32_1;
            input += 4;
            bytes = (input[0]+0u) | ((input[1]+0u)<< 8) | ((input[2]+0u)<<16) | ((input[3]+0u)<<24);
            v4 += bytes * XXH_PRIME32_2;
            v4  = XXH_rotl32(v4, 13);
            v4 *= XXH_PRIME32_1;
            input += 4;
        } while (input < limit);
        h = XXH_rotl32(v1, 1)  + XXH_rotl32(v2, 7)
          + XXH_rotl32(v3, 12) + XXH_rotl32(v4, 18);
    }
    else
    {
        h = seed + XXH_PRIME32_5;
    }
    h += len;
    len &= 15;
    while (len >= 4)
    {
        bytes = (input[0]+0u) | ((input[1]+0u)<< 8) | ((input[2]+0u)<<16) | ((input[3]+0u)<<24);
        h += bytes * XXH_PRIME32_3;
        input += 4;
        h  = XXH_rotl32(h, 17) * XXH_PRIME32_4;
        len -= 4;
    }
    while (len > 0)
    {
        h += (*input++) * XXH_PRIME32_5;
        h = XXH_rotl32(h, 11) * XXH_PRIME32_1;
        --len;
    }
    #undef XXH_rotl32
    h ^= h >> 15;
    h *= XXH_PRIME32_2;
    h ^= h >> 13;
    h *= XXH_PRIME32_3;
    h ^= h >> 16;
    return h;
}

inline constexpr void nplse__insertSkipnode(nanopulseDB *instance, unsigned key, int filepos)
{
    const int newNodeAddr = nplse__getNewNodePos(instance);
    instance->nodeVec[newNodeAddr].nodeid  = key;
    instance->nodeVec[newNodeAddr].filepos = filepos;
    
    // special case: key smaller than any before:
    if (key < instance->nodeVec[instance->headD].nodeid)
    {
        instance->nodeVec[newNodeAddr].next = instance->headD;
        instance->headD = newNodeAddr;
        return;
    }
    
    int current = instance->headD;
    int next = 0;
    for (int i=0; i<instance->nKeys-2; ++i)
    {
        next = instance->nodeVec[current].next;
        if (instance->nodeVec[next].nodeid > key)
        {
            instance->nodeVec[newNodeAddr].next = next;
            break;
        }
        current = next;
    }
    instance->nodeVec[current].next = newNodeAddr;
}

inline constexpr nplse__skipnode *nplse__findSkipnode(nanopulseDB *instance, const unsigned key, int *prev)
{
    int current = instance->headD;
    int p = current;
    for (int i=0; i<instance->nKeys; ++i)
    {
        if (instance->nodeVec[current].nodeid == key)
        {
            if (prev)
                *prev = p;
            return &instance->nodeVec[current];
        }
        p = current;
        current = instance->nodeVec[current].next;
    }
    return nullptr;
}




#if defined(COMP_AS_CPP)

constexpr unsigned nplse__testSkiplist()
{
    // setup:
    nplse__skipnode testNodes[32];
    nanopulseDB testDB{ .mappedfile=nullptr,
                        .szMappedfile=0,
                        .chunkSize=0,
                        .nodeVec=testNodes,
                        .seed=0
                      };
    // start testing:
    nplse__insertSkipnode(&testDB, 111, 0);
    const bool TEST_HEAD_IS_ZERO = testDB.headD == 0;
    nplse__insertSkipnode(&testDB, 333, 1);
    const bool TEST_NODE_ADD =  testNodes[0].nodeid == 111
                             && testNodes[1].nodeid == 333
                             && testNodes[testDB.headD].nodeid == 111
                             && testDB.headD == 0
                             && testNodes[0].next == 1
                             && testNodes[1].next == 0
                             && testNodes[testNodes[0].next].nodeid == 333;
    const bool TEST_NODES_POINTING_TO_CORRECT_POSITION =  testNodes[0].filepos == 0
                                                       && testNodes[1].filepos == 1;
    // insert node between:
    nplse__insertSkipnode(&testDB, 222, 2);
    const bool TEST_NODE_BETWEEN =  testNodes[2].nodeid == 222
                                 && testNodes[2].next == 1
                                 && testNodes[1].next == 0
                                 && testNodes[0].next == 2;
    // find nodes:
    int prevFirst=0, prevSecond=0, prevThird=0;
    const nplse__skipnode *first  = nplse__findSkipnode(&testDB, 111, &prevFirst);
    const nplse__skipnode *second = nplse__findSkipnode(&testDB, 333, &prevSecond);
    const nplse__skipnode *third  = nplse__findSkipnode(&testDB, 222, &prevThird);
    const bool TEST_NODES_FOUND =  first->filepos==0  && prevFirst==0
                                && second->filepos==1 && testNodes[prevSecond].nodeid==222
                                && third->filepos==2  && testNodes[prevThird].nodeid==111;
    // add node to end:
    nplse__insertSkipnode(&testDB, 444, 3);
    const bool TEST_NODE_AT_END =  nplse__findSkipnode(&testDB, 444, nullptr)->filepos == 3
                                && testNodes[3].nodeid == 444
                                && testNodes[1].next == 3
                                && testNodes[3].next == 0;
    // not found:
    const bool TEST_NODE_NOT_FOUND = nplse__findSkipnode(&testDB, 69, nullptr) == nullptr;
    
    
    return  TEST_HEAD_IS_ZERO
         | (TEST_NODE_ADD << 1)
         | (TEST_NODES_POINTING_TO_CORRECT_POSITION << 2)
         | (TEST_NODE_BETWEEN << 3)
         | (TEST_NODES_FOUND << 4)
         | (TEST_NODE_AT_END << 5)
         | (TEST_NODE_NOT_FOUND << 6)
         ;
}

static constexpr unsigned resList = nplse__testSkiplist();

static_assert(resList&  1, "head pointing to wrong place");
static_assert(resList&  2, "node was not correctly inserted");
static_assert(resList&  4, "nodes did not sit where they were supposed to");
static_assert(resList&  8, "nodes not correctly sorted");
static_assert(resList& 16, "nodes could not be recovered");
static_assert(resList& 32, "node at end not found");
static_assert(resList& 64, "nplse__findSkipnode() did not return NULL");


constexpr unsigned nplse__testDB()
{
}

static_assert(true);

#endif // defined(COMP_AS_CPP)

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
