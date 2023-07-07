/*
  You need to add '#define INCLUDEDB_IMPLEMENTATION' before including this header in ONE source file.
  Like this:
      #define INCLUDEDB_IMPLEMENTATION
      #include "includedb.h"
 
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
      
*/

#ifndef INCLUDEDB_DB_H
#define INCLUDEDB_DB_H

#ifndef INCLUDEDB_CONFIG_H
#define INCLUDEDB_CONFIG_H

#define INCLUDEDB_VER_MAJOR   0
#define INCLUDEDB_VER_MINOR   1
#define INCLUDEDB_VER_PATCH_A 0
#define INCLUDEDB_VER_PATCH_B 1
#define INCLUDEDB_VER_NUM ((INCLUDEDB_VER_MAJOR<<24) | (INCLUDEDB_VER_MINOR<<16) | (INCLUDEDB_VER_PATCH_A<<8) | INCLUDEDB_VER_PATCH_B)

#if defined(__cplusplus)
  #define COMPTIME constexpr
  #define CTOR2(x,y) constexpr x,y
  #define CTOR3(x,y,z) constexpr x,y,z
  #define CTOR4(w,x,y,z) constexpr w,x,y,z
#else
  #define nullptr ((void *)0)
  #define CTOR2(x,y)
  #define CTOR3(x,y,z)
  #define CTOR4(w,x,y,z)
  #define COMPTIME static const
  #define constexpr
  enum {false=0,true=1};
  typedef int bool;
  #ifndef DISABLE_TESTS
    #define DISABLE_TESTS
  #endif
#endif

// Error:
static const char *includedb__errorMsg;


#ifndef INCLUDEDB_MALLOC
  #include <stdlib.h>
  #define includedb__malloc(sz)          malloc(sz)
  #define includedb__realloc(p,newsz)    realloc(p,newsz)
  #define includedb__zeroalloc(cnt,sz)   calloc(cnt,sz)
  #define includedb__free(p)             free(p)
  #define INCLUDEDB_MALLOC
#endif


#ifndef INCLUDEDB_HASH
  #define INCLUDEDB_HASH(key,keylen,seed)   includedb__xx32(key,keylen,seed)
#endif


struct includedb__file;

bool includedb__fileOpen(struct includedb__file *file, const char *filename);
bool includedb__fileCreate(struct includedb__file *file, const char *filename);
bool includedb__fileClose(struct includedb__file *file);
int  includedb__fileWrite(struct includedb__file *file, const unsigned char *bytes, int len, int filepos);
void includedb__fileRead(struct includedb__file *file, unsigned char *bytes, int len, int filepos);
bool includedb__fileGrow(struct includedb__file *file, int len);

#define INCLUDEDB_USE_STD_FILE_FALLBACK // todo: temporary. This should be removed after better file-acces is iimplemented
#if defined(INCLUDEDB_USE_STD_FILE_FALLBACK)
  #include <stdio.h>
  typedef struct includedb__file
  {
      FILE *pFile;
  } includedb__file;

  bool includedb__fileOpen(includedb__file *file, const char *filename)
  {
      file->pFile = fopen(filename, "rb+");
      return !!file->pFile;
  }

  bool includedb__fileCreate(includedb__file *file, const char *filename)
  {
      file->pFile = fopen(filename, "wb+");
      return !!file->pFile;
  }

  bool includedb__fileClose(includedb__file *file)
  {
      return fclose(file->pFile);
  }

  int  includedb__fileWrite(includedb__file *file, const unsigned char *bytes, int len, int filepos)
  {
      fseek(file->pFile, filepos, SEEK_SET);
      return fwrite(bytes, sizeof(unsigned char), len, file->pFile);
  }

  void includedb__fileRead(includedb__file *file, unsigned char *bytes, int len, int filepos)
  {
      fseek(file->pFile, filepos, SEEK_SET);
      const int r = fread(bytes, sizeof(unsigned char), len, file->pFile);
      (void)r;
  }

  bool includedb__fileGrow(includedb__file *file, int len)
  {
      // ¯\_(ツ)_/¯
      (void)file;
      (void)len;
      return true;
  }
#endif // defined(INCLUDEDB_USE_STD_FILE_FALLBACK)

COMPTIME int includedb__header_keyhashLen        = 4;
COMPTIME int includedb__header_keylenLen         = 4;
COMPTIME int includedb__header_vallenLen         = 4;
COMPTIME int includedb__header_recordPriorityLen = 4;


#if !defined(INCLUDEDB_CHUNK_SIZE)
  #define INCLUDEDB_CHUNK_SIZE 256
#endif


struct includeDB;

typedef int (*pincludedb__write)(struct includeDB *instance, int location, int amount);
typedef int (*pincludedb__read)(struct includeDB *instance, int location, int amount);

enum includedb__errorCodes
{
    INCLUDEDB__OK,
    INCLUDEDB__BITVEC_ALLOC,
    INCLUDEDB__NODE_ALLOC,
    INCLUDEDB__SLOTS_ALLOC,
    INCLUDEDB__BUFFER_ALLOC,
    INCLUDEDB__ALREADY_KEY,
    INCLUDEDB__KEY_NOT_FOUND
};


struct includedb__bitvec;

typedef int (*pincludedb__bitvecAlloc)(struct includedb__bitvec *bitvec, int amount);

typedef struct includedb__bitvec
{
    CTOR3(includedb__bitvec() : bitvec(nullptr), szVecIn32Chunks(0), includedb__bitvecAlloc(nullptr) {})
    unsigned *bitvec;
    int szVecIn32Chunks;
    pincludedb__bitvecAlloc includedb__bitvecAlloc;
} includedb__bitvec;


typedef struct includedb__skipnode
{
    CTOR4(includedb__skipnode() : nodeid(0), filepos(0), next(0), visits(0) {})
    unsigned nodeid;
    int filepos;
    int next; // pointer into 'nodeVec'
    int visits;
} includedb__skipnode;

typedef int (*pincludedb__nodevecAlloc)(struct includeDB *instance, int newSize);

typedef struct includeDB
{
    // Buffer/Address:
    union
    {
        unsigned char *buffer;
        unsigned char *address;
    };
    int szBuf;

    includedb__bitvec occupied;
     
    int chunkSize;
    int globalVisits;

    // List:
    includedb__skipnode *nodeVec;
    int headA, headB, headC, headD;
    int nKeys;
    int nAllocated;
    pincludedb__nodevecAlloc includedb__nodevecAlloc;
    
    // Hashing:
    unsigned seed;

    // File:
    union
    {
        includedb__file file;
        unsigned char *mappedArray;
    };
    pincludedb__write includedb__write;
    pincludedb__read includedb__read;

    int cursor;

    // Error code
    enum includedb__errorCodes ec;
} includeDB;

static constexpr int includedb_ok = 0;
static constexpr int includedb_error = 1;




/*
 ------------------------------------------------------------------------------
    Public interface
    All operations are synchronous!! 
 ------------------------------------------------------------------------------
*/
// Add a new record, returns includedb_error on fail
static constexpr int includedb_put(includeDB *instance, const unsigned char *key, int keylen, const unsigned char *val, int vallen);

// Get a pointer to an existing record. *vallen contains the size of the returned value 
// in bytes and can be NULL if you are not interested in it. Calling this is going to 
// invalidate previously retuned pointers
static constexpr unsigned char *includedb_get(includeDB *instance, const unsigned char *key, int keylen, int *vallen);

// Upon opening a database a cursor is pointing to the first record. Use this to move 
// the cursor to the next record
static constexpr void includedb_next(includeDB *instance);

// Get the key at cursor position. Optionally the size of the key can be stored in *keylen. Pass NULL if this is not needed
static constexpr unsigned char *includedb_curGetKey(includeDB *instance, int *keylen);

// Open existing, or create new
static includeDB *includedb_open(const char *filename);

// Close. Must be called to ensure all changes are written to disk
static void includedb_close(includeDB *instance);

// Get error
static const char *includedb_getError(includeDB *instance);



/*++++++++++++++++++++++++++++++++++++++*/
/*               hashfunction (XX hash) */
/* (default in case the user didn't     */
/* provide one))                        */
/*++++++++++++++++++++++++++++++++++++++*/
static constexpr unsigned includedb__xx32(const unsigned char *input, int len, unsigned seed)
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


#endif // INCLUDEDB_CONFIG_H


#ifdef INCLUDEDB_IMPLEMENTATION


#ifndef INCLUDEDB_CORE_H
#define INCLUDEDB_CORE_H




#endif // INCLUDEDB_CORE_H



typedef unsigned includedb__inttype;

#define N_SLOTS (sizeof(includedb__inttype) * 8)


typedef struct includedb__bloom
{
    CTOR2(includedb__bloom() : counters{0}, bitmap(0) {})
    int counters[N_SLOTS];

    includedb__inttype bitmap;
} includedb__bloom;


constexpr void includedb__bloomPut(includedb__bloom *bloom, includedb__inttype hash)
{
    bloom->bitmap |= hash;
    for (int i=0; i<N_SLOTS; i+=8)
    {
        bloom->counters[i+0] += hash & 1;  hash >>= 1;
        bloom->counters[i+1] += hash & 1;  hash >>= 1;
        bloom->counters[i+2] += hash & 1;  hash >>= 1;
        bloom->counters[i+3] += hash & 1;  hash >>= 1;
        bloom->counters[i+4] += hash & 1;  hash >>= 1;
        bloom->counters[i+5] += hash & 1;  hash >>= 1;
        bloom->counters[i+6] += hash & 1;  hash >>= 1;
        bloom->counters[i+7] += hash & 1;  hash >>= 1;
    }
}

constexpr void includedb__bloomRemove(includedb__bloom *bloom, includedb__inttype hash)
{
    includedb__inttype mask = 0;
    for (int i=0; i<N_SLOTS; i+=8)
    {
        bloom->counters[i+0] -= hash & 1; hash >>= 1;
        bloom->counters[i+1] -= hash & 1; hash >>= 1;
        bloom->counters[i+2] -= hash & 1; hash >>= 1;
        bloom->counters[i+3] -= hash & 1; hash >>= 1;
        bloom->counters[i+4] -= hash & 1; hash >>= 1;
        bloom->counters[i+5] -= hash & 1; hash >>= 1;
        bloom->counters[i+6] -= hash & 1; hash >>= 1;
        bloom->counters[i+7] -= hash & 1; hash >>= 1;
        mask |= !!bloom->counters[i+0] << (i  );
        mask |= !!bloom->counters[i+1] << (i+1);
        mask |= !!bloom->counters[i+2] << (i+2);
        mask |= !!bloom->counters[i+3] << (i+3);
        mask |= !!bloom->counters[i+4] << (i+4);
        mask |= !!bloom->counters[i+5] << (i+5);
        mask |= !!bloom->counters[i+6] << (i+6);
        mask |= !!bloom->counters[i+7] << (i+7);
    }
    bloom->bitmap = bloom->bitmap & mask;
}

constexpr bool includedb__bloomMaybehave(includedb__bloom *bloom, includedb__inttype hash)
{
    return !((hash&bloom->bitmap) ^ hash);
}


/*++++++++++++++++++++++++++++++++++++++*/
/*                                Tests */
/*++++++++++++++++++++++++++++++++++++++*/
#if !defined(DISABLE_TESTS)

constexpr unsigned includedb_testBloom()
{
    includedb__bloom bloomTest;

    includedb__bloomPut(&bloomTest, 0b0110);
    const bool TEST_PUT_OK = includedb__bloomMaybehave(&bloomTest, 0b0110);
    const bool TEST_SURE_DOESNT_HAVE = includedb__bloomMaybehave(&bloomTest, 0b0001) == false;
    const bool TEST_OVERLAPPING_BITS = includedb__bloomMaybehave(&bloomTest, 0b0100) == true;
    const bool TEST_CONFLICTING_BITS = includedb__bloomMaybehave(&bloomTest, 0b1100) == false;
    includedb__bloomPut(&bloomTest, 0b1111);
    includedb__bloomRemove(&bloomTest, 0b0110);
    const bool TEST_KEY_FOUND_AFTER_REMOVE = includedb__bloomMaybehave(&bloomTest, 0b0110) == true;
    includedb__bloomRemove(&bloomTest, 0b1111);
    const bool TEST_KEY_NOT_FOUND_AFTER_REMOVE = includedb__bloomMaybehave(&bloomTest, 0b1111) == false;    

    return TEST_PUT_OK
        | (TEST_SURE_DOESNT_HAVE << 1)
        | (TEST_OVERLAPPING_BITS << 2)
        | (TEST_CONFLICTING_BITS << 3)
        | (TEST_KEY_FOUND_AFTER_REMOVE << 4)
        | (TEST_KEY_NOT_FOUND_AFTER_REMOVE << 5)
        ;
}
static constexpr unsigned resBloomtest = includedb_testBloom();

static_assert(resBloomtest& 1, "Didn't work");
static_assert(resBloomtest& 2, ".maybeHave() returned 'true' when the key clearly didn't exist");
static_assert(resBloomtest& 4, ".maybeHave() overlapping bits");
static_assert(resBloomtest& 8, ".maybeHave() conflicting bits");
static_assert(resBloomtest&16, "key disappeard after remove");
static_assert(resBloomtest&32, "key should have disappeared after remove");

#endif // !defined(DISABLE_TESTS)




static int includedb__bitvecAlloc(includedb__bitvec *bitvec, int amount)
{
    const int szOld = bitvec->szVecIn32Chunks;
    bitvec->szVecIn32Chunks += amount;
    unsigned *tmp = (unsigned *)includedb__zeroalloc(bitvec->szVecIn32Chunks, sizeof(unsigned));
    if (!tmp)
        return includedb_error;
    for (int i=0; i<szOld; ++i)
        tmp[i] = bitvec->bitvec[i];
    includedb__free(bitvec->bitvec);
    bitvec->bitvec = tmp;
    return includedb_ok;
}

static constexpr unsigned includedb__bitvecCheck(const includedb__bitvec *bitvec, int pos)
{
    const unsigned bit = bitvec->bitvec[pos>>5];
    return (bit >> (pos&31)) & 1;
}

static constexpr void includedb__bitvecSet(includedb__bitvec *bitvec, int pos)
{
    const unsigned bit = 1 << (pos&31);
    bitvec->bitvec[pos>>5] |= bit;
}


/*++++++++++++++++++++++++++++++++++++++*/
/*                                Tests */
/*++++++++++++++++++++++++++++++++++++++*/
#if !defined(DISABLE_TESTS)

constexpr unsigned includedb__testBitvec()
{
    unsigned bitvecBits[3] = {0};
    includedb__bitvec testBitvec;
    testBitvec.bitvec = bitvecBits;
    for (int i=0; i<64; ++i)
        includedb__bitvecSet(&testBitvec, i);
    const bool TEST_BIT_63 =   includedb__bitvecCheck(&testBitvec, 63)
                           && !includedb__bitvecCheck(&testBitvec, 64);
    includedb__bitvecSet(&testBitvec, 64);
    const bool TEST_BIT_64 =   includedb__bitvecCheck(&testBitvec, 64)
                           && !includedb__bitvecCheck(&testBitvec, 65);
    
    return  TEST_BIT_63
         | (TEST_BIT_64 << 1);
}
constexpr unsigned resBitvec = includedb__testBitvec();

static_assert(resBitvec&1, "bit 63 not correct");
static_assert(resBitvec&2, "bit 64 not correct");

#endif // !defined(DISABLE_TESTS)




/*++++++++++++++++++++++++++++++++++++++*/
/*                                slots */
/*++++++++++++++++++++++++++++++++++++++*/
static constexpr int includedb__gatherSlots(includeDB *instance, int requiredSlots)
{
    bool haveAvail = false;
    int location = 0;
    includedb__bitvec *bitvec = &instance->occupied;
    for (; location<((bitvec->szVecIn32Chunks*32)-requiredSlots) && !haveAvail; ++location)
        if (includedb__bitvecCheck(bitvec, location) == 0)
        {
            haveAvail = true;
            for (int i=1; i<requiredSlots-1; ++i)
                haveAvail = haveAvail && (includedb__bitvecCheck(bitvec, location+i) == 0);
        }
    location -= 1;
    if (!haveAvail)
    {
        // make room:
        const int amount = (requiredSlots/32) + 1;
        if (bitvec->includedb__bitvecAlloc(bitvec, amount))
        {
            instance->ec = INCLUDEDB__BITVEC_ALLOC;
            return -1; // failed
        }
        // todo : grow file 32*chunksz
        return includedb__gatherSlots(instance, requiredSlots);
    }
    return location;
}

static constexpr void includedb__markSlots(includedb__bitvec *bitvec, int start, int len)
{
    for (int i=0; i<len; i++)
        includedb__bitvecSet(bitvec, start+i);
}

static int includedb__nodevecAlloc(includeDB *instance, int newSize)
{
    includedb__skipnode *tmp = (includedb__skipnode *)includedb__realloc(instance->nodeVec, newSize * sizeof(includedb__skipnode));
    if (!tmp)
        return includedb_error;
    instance->nodeVec = tmp;
    return includedb_ok;
}


/*++++++++++++++++++++++++++++++++++++++*/
/*                       skiplist impl. */
/*++++++++++++++++++++++++++++++++++++++*/
static constexpr int includedb__getNewKeyPos(includeDB *instance)
{
    if (instance->nKeys == instance->nAllocated)
    {
        const int newSize = instance->nAllocated << 1;
        if (instance->includedb__nodevecAlloc(instance, newSize) == 1)
        {
            instance->ec = INCLUDEDB__NODE_ALLOC;
            return instance->nKeys;
        }
        instance->nAllocated = newSize;
    }
    return instance->nKeys++;
}

static constexpr void includedb__insertSkipnode(includeDB *instance, unsigned key, int pos, int layer)
{
    // special case: key smaller than any before:
    if (key < instance->nodeVec[instance->headD].nodeid)
    {
        instance->nodeVec[pos].next = instance->headD;
        instance->headD = pos;
        return;
    }
    
    int current = instance->headD;
    int next = 0;
    for (int i=0; i<instance->nKeys-2; ++i)
    {
        next = instance->nodeVec[current].next;
        if (instance->nodeVec[next].nodeid > key)
        {
            instance->nodeVec[pos].next = next;
            break;
        }
        else if (current == next)
            i = instance->nKeys; // break
        current = next;
    }
    instance->nodeVec[current].next = pos;
}

static constexpr void includedb__insertNewSkipnode(includeDB *instance, unsigned key, int filepos)
{
    const int newNodeAddr = includedb__getNewKeyPos(instance);
    instance->nodeVec[newNodeAddr].nodeid  = key;
    instance->nodeVec[newNodeAddr].filepos = filepos;
    
    includedb__insertSkipnode(instance, key, newNodeAddr, 3);
}

static constexpr int includedb__findPrevSkipnode(includeDB *instance, const unsigned key, const int start, const int layer)
{
    int current = start;
    int prev = current;
    for (int i=0; i<instance->nKeys; ++i)
    {
        if (layer<3 && (prev==current || instance->nodeVec[current].nodeid>key))
            return includedb__findPrevSkipnode(instance, key, instance->nodeVec[current].next, layer+1);
        else if (instance->nodeVec[current].nodeid == key)
        {
            instance->nodeVec[current].visits += 1;
            instance->globalVisits += 1;
            //if (((instance->nodeVec[current].visits*100) / instance->globalVisits) > 20)
                //icldb__insertSkipnode(instance, key, current, layer-1);
            return current;
        }
        else if (instance->nodeVec[current].nodeid >= key)
            break;
        prev = current;
        current = instance->nodeVec[current].next;
    }
    return prev;
}

static constexpr includedb__skipnode *includedb__findSkipnode(includeDB *instance, const unsigned key)
{
    //const int res = icldb__findPrevSkipnode(instance, key, instance->headA, 0);
    const int res = includedb__findPrevSkipnode(instance, key, instance->headD, 3);
    return instance->nodeVec[res].nodeid == key ? &instance->nodeVec[res] : nullptr;
}


/*++++++++++++++++++++++++++++++++++++++*/
/*                                Tests */
/*++++++++++++++++++++++++++++++++++++++*/
#if !defined(DISABLE_TESTS)

constexpr unsigned includedb__testSlots()
{
    includeDB testDB{ .mappedArray=nullptr,
                      .chunkSize=0,
                      .nodeVec=nullptr,
                      .seed=0
                    };
    unsigned bitvecBits[3/* *32 */] = {0};
    testDB.occupied.bitvec = bitvecBits;
    testDB.occupied.szVecIn32Chunks  = 1;
    auto testAlloc = [](includedb__bitvec *bitvec, int amount)->int
                     {
                         if ((bitvec->szVecIn32Chunks+amount) <= 3)
                         {
                             bitvec->szVecIn32Chunks += amount;
                             return 0;
                         }
                         return 1;
                     };
    testDB.occupied.includedb__bitvecAlloc = testAlloc;
    const bool TEST_HAVE_ZERO_SLOTS = includedb__gatherSlots(&testDB, 0) == 0;
    int location = includedb__gatherSlots(&testDB, 2);
    includedb__markSlots(&testDB.occupied, location, 2);
    const bool TEST_TWO_SLOTS_OCCUPIED =  (bitvecBits[0]&1)
                                       && (bitvecBits[0]&2)
                                       && !(bitvecBits[0]&4)
                                       && location==0;
    location = includedb__gatherSlots(&testDB, 1);
    includedb__markSlots(&testDB.occupied, location, 1);
    const bool TEST_LOCATION_TWO = location == 2;
    location = includedb__gatherSlots(&testDB, 7);
    includedb__markSlots(&testDB.occupied, location, 7);
    const bool TEST_LOCATION_THREE = location == 3;
    // test large allocation:
    location = includedb__gatherSlots(&testDB, 24+39);
    includedb__markSlots(&testDB.occupied, location, 24+39);
    const bool TEST_LOCATION_EIGHT = location == 10;
    location = includedb__gatherSlots(&testDB, 22);
    includedb__markSlots(&testDB.occupied, location, 22);
    const bool TEST_LOCATION_DEEP = location == 10+24+39;
    const bool TEST_ALL_FLAGS_SET =  !(bitvecBits[0]^0xffffffff)
                                  && !(bitvecBits[1]^0xffffffff)
                                  && !(bitvecBits[2]^0x7fffffff)
    //&& !(bitvecBits[2]^0b00000000000000000000000011111111)
    //&& !(bitvecBits[2]^0b1111111111111111111111111111111)
    //&& bitvecBits[2] > 0x1fffffffu
    ;
    
    // todo: test free slots!
    
    
    return  TEST_HAVE_ZERO_SLOTS
         | (TEST_TWO_SLOTS_OCCUPIED<< 1)
         | (TEST_LOCATION_TWO<< 2)
         | (TEST_LOCATION_THREE<< 3)
         | (TEST_LOCATION_EIGHT<< 4)
         | (TEST_LOCATION_DEEP<< 5)
         | (TEST_ALL_FLAGS_SET<< 6)
         ;
}
constexpr unsigned resSlots = includedb__testSlots();

static_assert(resSlots&  1, "couldn't gather 0 slots");
static_assert(resSlots&  2, "slots were not correctly marked as 'occupied'");
static_assert(resSlots&  4, "incorrect slot position (2)");
static_assert(resSlots&  8, "incorrect slot position (3)");
static_assert(resSlots& 16, "incorrect slot position (8)");
static_assert(resSlots& 32, "incorrect slot position (8+24+39)");
static_assert(resSlots& 64, "flags set incorrectly");


constexpr unsigned includedb__testSkiplist()
{
    // setup:
    includedb__skipnode testNodes[32];
    includeDB testDB{ .mappedArray=nullptr,
                      .chunkSize=0,
                      .nodeVec=testNodes,
                      .includedb__nodevecAlloc=[](includeDB *instance, int newSize)->int
                                               {
                                                   (void)instance;
                                                   (void)newSize;
                                                   return 0;
                                               },
                      .seed=0
                    };
    // start testing:
    includedb__insertNewSkipnode(&testDB, 111, 0);
    const bool TEST_HEAD_IS_ZERO = testDB.headD == 0;
    includedb__insertNewSkipnode(&testDB, 333, 1);
    const bool TEST_NODE_ADD =  testNodes[0].nodeid == 111
                             && testNodes[testDB.headD].nodeid == 111
                             && testDB.headD == 0
                             && testNodes[0].next == 1
                             && testNodes[1].next == 0
                             && testNodes[testNodes[0].next].nodeid == 333;
    const bool TEST_NODES_POINTING_TO_CORRECT_POSITION =  testNodes[0].filepos == 0
                                                       && testNodes[1].filepos == 1;
    // insert node between:
    includedb__insertNewSkipnode(&testDB, 222, 2);
    const bool TEST_NODE_BETWEEN =  testNodes[2].nodeid == 222
                                 && testNodes[2].next == 1
                                 && testNodes[1].next == 0
                                 && testNodes[0].next == 2;
    // find nodes:
    const includedb__skipnode *first  = includedb__findSkipnode(&testDB, 111);
    const includedb__skipnode *second = includedb__findSkipnode(&testDB, 333);
    const includedb__skipnode *third  = includedb__findSkipnode(&testDB, 222);
    const bool TEST_NODES_FOUND =  first && second && third
                                && first->filepos==0 && second->filepos==1 && third->filepos==2;
    // add node to end:
    includedb__insertNewSkipnode(&testDB, 444, 3);
    const includedb__skipnode *found = includedb__findSkipnode(&testDB, 444);
    const bool TEST_NODE_AT_END =  found && found->filepos == 3
                                && testNodes[3].nodeid == 444
                                && testNodes[1].next == 3
                                && testNodes[3].next == 0;
    // not found:
    const bool TEST_NODE_NOT_FOUND = includedb__findSkipnode(&testDB, 69) == nullptr;
    
    //todo test remove
    
    return  TEST_HEAD_IS_ZERO
         | (TEST_NODE_ADD << 1)
         | (TEST_NODES_POINTING_TO_CORRECT_POSITION << 2)
         | (TEST_NODE_BETWEEN << 3)
         | (TEST_NODES_FOUND << 4)
         | (TEST_NODE_AT_END << 5)
         | (TEST_NODE_NOT_FOUND << 6)
         ;
}

constexpr unsigned resList = includedb__testSkiplist();

static_assert(resList&  1, "head pointing to wrong place");
static_assert(resList&  2, "node was not correctly inserted");
static_assert(resList&  4, "nodes did not sit where they were supposed to");
static_assert(resList&  8, "nodes not correctly sorted");
static_assert(resList& 16, "nodes could not be recovered");
static_assert(resList& 32, "node at end not found");
static_assert(resList& 64, "includedb__findSkipnode() did not return NULL");

#endif // !defined(DISABLE_TESTS)






static constexpr int includedb__dbBufferResize(includeDB *instance, int newsize)
{
    const int sz = instance->szBuf;
    if (sz < newsize)
    {
        instance->szBuf = ((newsize/sz) * sz) + sz;
        unsigned char *tmp = (unsigned char *)includedb__realloc(instance->buffer, instance->szBuf);
        if (!tmp)
        {
            instance->ec = INCLUDEDB__BUFFER_ALLOC;
            return includedb_error;
        }
        instance->buffer = tmp;
    }
    return includedb_ok;
}

static int includedb__dbWrite(includeDB *instance, int location, int amount)
{
    location += 128; // db header size
    const int res = includedb__fileWrite(&instance->file, instance->buffer, amount, location);
    return res != amount;
}

static int includedb__dbRead(includeDB *instance, int location, int amount)
{
    location += 128;
    
    // make sure the buffer is big enough:
    if (includedb__dbBufferResize(instance, amount) == includedb_error)
        return includedb_error;
    
    includedb__fileRead(&instance->file, instance->buffer, amount, location);
    return includedb_ok;
}





static constexpr int includedb_put(includeDB *instance, const unsigned char *key, int keylen, const unsigned char *val, int vallen)
{
    const unsigned keyhash = INCLUDEDB_HASH(key, keylen, instance->seed);
    if (includedb_get(instance, key, keylen, nullptr))
    {
        instance->ec = INCLUDEDB__ALREADY_KEY;
        return includedb_error;
    }
    const int chunkSize = instance->chunkSize;
    const int requiredSizeInByte = includedb__header_keyhashLen
                                 + includedb__header_keylenLen
                                 + includedb__header_vallenLen
                                 + includedb__header_recordPriorityLen
                                 + keylen
                                 + vallen
                                 + 1;
    const int requiredSlots = (requiredSizeInByte/chunkSize) + 1; // '!!(requiredSizeInByte%chunkSize)' easier to just add 1
    const int requiredSizeOfRecord = requiredSlots*chunkSize;
    const int location = includedb__gatherSlots(instance, requiredSlots);
    if (location == -1)
    {
        instance->ec = INCLUDEDB__SLOTS_ALLOC;
        return includedb_error;
    }
    includedb__markSlots(&instance->occupied, location, requiredSlots);
    // Hook up new node:
    includedb__insertNewSkipnode(instance, keyhash, location*chunkSize);
    // Write data:
    if (includedb__dbBufferResize(instance, requiredSizeOfRecord))
        return includedb_error;
    unsigned char *data = instance->buffer;
    data[ 0] = (keyhash>>24) & 0xff;
    data[ 1] = (keyhash>>16) & 0xff;
    data[ 2] = (keyhash>> 8) & 0xff;
    data[ 3] = (keyhash    ) & 0xff;
    data[ 4] = (keylen>>24) & 0xff;
    data[ 5] = (keylen>>16) & 0xff;
    data[ 6] = (keylen>> 8) & 0xff;
    data[ 7] = (keylen    ) & 0xff;
    data[ 8] = (vallen>>24) & 0xff;
    data[ 9] = (vallen>>16) & 0xff;
    data[10] = (vallen>> 8) & 0xff;
    data[11] = (vallen    ) & 0xff;
    // Priority field is little endian and initialized to 1 for a new record.
    // If 0 then it is a tombstone marked for overwrite:
    data[12] = 0;
    data[13] = 0;
    data[14] = 0;
    data[15] = 1;
    data += 16;
    // key first:
    for (int i=0; i<keylen; ++i, data++)
        *data = key[i];
    // then data:
    for (int i=0; i<vallen; ++i, data++)
        *data = val[i];
    // then write:
    return instance->includedb__write(instance, location*chunkSize, requiredSizeOfRecord);
}

static constexpr unsigned char *includedb_get(includeDB *instance, const unsigned char *key, int keylen, int *vallen)
{
    instance->ec = INCLUDEDB__KEY_NOT_FOUND;
    const unsigned keyhash = INCLUDEDB_HASH(key, keylen, instance->seed);
   // if (includedb__bloomMaybehave(instance, keyhash) == false) // todo impl.
     //   return nullptr;
        
    // todo increase priority
    
    COMPTIME unsigned headerSize = includedb__header_keyhashLen
                                 + includedb__header_keylenLen
                                 + includedb__header_vallenLen
                                 + includedb__header_recordPriorityLen;
    includedb__skipnode *found = includedb__findSkipnode(instance, keyhash);
    if (found)
    {
        // read header
        instance->includedb__read(instance, found->filepos, headerSize);
        unsigned char *data = &instance->buffer[includedb__header_keyhashLen];
        const unsigned kl = (data[0]<<24) | (data[1]<<16) | (data[2]<< 8) | data[3];
        data += includedb__header_keylenLen;
        const unsigned vl = (data[0]<<24) | (data[1]<<16) | (data[2]<< 8) | data[3];
        // read rest
        instance->includedb__read(instance, found->filepos+headerSize + kl, vl);
        int unusedVallen = 0; // Dummy, in case vallen == nullptr
        vallen = vallen ? : &unusedVallen;
        *vallen = vl;
        instance->ec = INCLUDEDB__OK;
        return instance->buffer;
    }
    return nullptr;
}

static constexpr void includedb_next(includeDB *instance)
{
    instance->cursor += 1;
}

static constexpr unsigned char *includedb_curGetKey(includeDB *instance, int *keylen)
{
    *keylen = 0;
    if (instance->cursor == instance->nKeys)
        return nullptr;
    COMPTIME int headerSize = includedb__header_keyhashLen
                            + includedb__header_keylenLen
                            + includedb__header_vallenLen
                            + includedb__header_recordPriorityLen;
    const int filepos = instance->nodeVec[instance->cursor].filepos;
    if (instance->includedb__read(instance, filepos, headerSize) != 0)
        return nullptr;
    const unsigned char *data = instance->buffer;
    const int kl = (data[4]<<24) | (data[5]<<16) | (data[6]<<8) | data[7];
    if (instance->includedb__read(instance, filepos+headerSize, kl) != 0)
        return nullptr;
    *keylen = kl;
    return instance->buffer;
}

static includeDB *includedb_open(const char *filename)
{
    COMPTIME unsigned char magic[] = "incldedb";
    COMPTIME unsigned v = INCLUDEDB_VER_NUM;
    COMPTIME unsigned char versionStr_BE[] = {v&0xff,(v>>8)&0xff,(v>>16)&0xff,(v>>24)&0xff};
    includeDB *newInstance = (includeDB *)includedb__malloc(sizeof(includeDB));
    if (!newInstance)
    {
        includedb__errorMsg = "Couldn't alloc db. Out of mem?";
        return nullptr;
    }
    
    int nKeys = 0;
    if (!includedb__fileOpen(&newInstance->file, filename))
    {
        if (!includedb__fileCreate(&newInstance->file, filename))
        {
            includedb__errorMsg = "Couldn't open db. File corrupted?";
            // Cannot call close() on an instance that hasn't been created correctly.
            // Return NULL instead:
            includedb__free(newInstance);
            return nullptr;
        }
        // write signature
        includedb__fileWrite(&newInstance->file, magic, 8, 0);
        // write version
        includedb__fileWrite(&newInstance->file, versionStr_BE, 4, 8);
        // write chunksize
        COMPTIME unsigned cs = INCLUDEDB_CHUNK_SIZE;
        COMPTIME unsigned char chunksize[] = {(cs>>24)&0xff, (cs>>16)&0xff, (cs>>8)&0xff, cs&0xff};
        int written = includedb__fileWrite(&newInstance->file, chunksize, 4, 12);
        newInstance->chunkSize = cs;
        // write keys & "visits"
        COMPTIME unsigned char keysNvisits[] = {0,0,0,0,0,0,0,0};
        includedb__fileWrite(&newInstance->file, keysNvisits, 8, 16);
        newInstance->globalVisits = 0;
        // create seed
        const unsigned sd = 6987; // todo get from time
        const unsigned char seedStr[] = {(sd>>24)&0xff, (sd>>16)&0xff, (sd>>8)&0xff, sd&0xff};
        includedb__fileWrite(&newInstance->file, seedStr, 4, 24);
        newInstance->seed = sd;
        (void)written;
    }
    else
    // open existing file:
    {
        // read db
        unsigned char buf[28];
        includedb__fileRead(&newInstance->file, buf, 28, 0);
        #define k(p,c) (buf[p]==c)
        bool ok=k(0,'i') && k(1,'n') && k(2,'c') && k(3,'l') && k(4,'d') && k(5,'e') && k(6,'d') && k(7,'b');
        if (!ok)
        {
            includedb__errorMsg = "Couldn't open db. File not recognized";
            includedb__free(newInstance);
            return nullptr;
        }
        // read version vs minimum required
        ok =       buf[11] == INCLUDEDB_VER_MAJOR;
        ok = ok && buf[10] <= INCLUDEDB_VER_MINOR;
        ok = ok && buf[ 9] <= INCLUDEDB_VER_PATCH_A;
        ok = ok && buf[ 8] <= INCLUDEDB_VER_PATCH_B;
        if (!ok)
        {
            // todo upgrade version if possible!
            
            includedb__errorMsg = "Couldn't open db. Incompatible version";
            includedb__free(newInstance);
            return nullptr;
        }
        // read chunksise
        newInstance->chunkSize = (buf[12]<<24) | (buf[13]<<16) | (buf[14]<<8) | buf[15];
        // read # of keys
        nKeys = (buf[16]<<24) | (buf[17]<<16) | (buf[18]<<8) | buf[19];
        // read global visits
          // todo: not yet
        // read randseed
        newInstance->seed = (buf[24]<<24) | (buf[25]<<16) | (buf[26]<<8) | buf[27];
        
        // chunkSize =
        // read filesz
        // read vists
        
    }
    // Create buffer:
    newInstance->buffer = (unsigned char *)includedb__malloc(newInstance->chunkSize);
    newInstance->szBuf = 1;
    // Write fn:
    newInstance->includedb__write = includedb__dbWrite;
    // Read fn:
    newInstance->includedb__read = includedb__dbRead;
    // setup bitvec:
    newInstance->occupied.bitvec = (unsigned *)includedb__zeroalloc(1, sizeof(unsigned));
    newInstance->occupied.szVecIn32Chunks = 1;
    newInstance->occupied.includedb__bitvecAlloc = includedb__bitvecAlloc;
    // setup node list:
    COMPTIME int nInitialBits = sizeof(unsigned) * 8;
    newInstance->nodeVec = (includedb__skipnode *)includedb__malloc(sizeof(includedb__skipnode) * nInitialBits);
    newInstance->nAllocated = nInitialBits;
    newInstance->includedb__nodevecAlloc = includedb__nodevecAlloc;
    
    // todo: init heads
    newInstance->headD = 0;
    
    
    newInstance->nKeys = 0;
    // put cursor to the start
    newInstance->cursor = 0;
    // init bloomfilter (todo)
    //newInstance->bloommap = 0;
    //for (int i=0; i<szBloommap; ++i)
    //    newInstance->bloomcounters[i] = 0;
    // reset error
    newInstance->ec = INCLUDEDB__OK;
    
    
    
    // Build index:
    COMPTIME int dbHeaderSize = 128;
    int offset = 0;
    unsigned char buf[20];
    for (int i=0; i<nKeys; ++i)
    {
        includedb__fileRead(&newInstance->file, buf, 16, offset+dbHeaderSize);
        const unsigned keyhash = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
        includedb__insertNewSkipnode(newInstance, keyhash, offset);
        
        
        const unsigned priA=buf[12], priB=buf[13], priC=buf[14], priD=buf[15];
        const unsigned tombstone = (priA<<24) | (priB<<16) | (priC<< 8) | priD;
        
        
        // get position for the next record:
        const unsigned keylen = (buf[ 4]<<24) | (buf[ 5]<<16) | (buf[ 6]<<8) | buf[ 7];
        const unsigned vallen = (buf[ 8]<<24) | (buf[ 9]<<16) | (buf[10]<<8) | buf[11];
        const int requiredSizeInByte = includedb__header_keyhashLen
                                     + includedb__header_keylenLen
                                     + includedb__header_vallenLen
                                     + includedb__header_recordPriorityLen
                                     + keylen
                                     + vallen
                                     + 1;
        const int requiredSlots = (requiredSizeInByte/newInstance->chunkSize) + 1;
        const int slotLocation = includedb__gatherSlots(newInstance, requiredSlots);
        includedb__markSlots(&newInstance->occupied, slotLocation, requiredSlots);
        offset += newInstance->chunkSize * requiredSlots;
    }
    
    return newInstance;
}

static void includedb_close(includeDB *instance)
{
    if (instance == nullptr)
        return;
        
    unsigned char keysNvisits[8] = {0};
    const int nk = instance->nKeys;
    keysNvisits[0]=nk>>24; 
    keysNvisits[1]=(nk>>16)&0xff; 
    keysNvisits[2]=(nk>>8)&0xff; 
    keysNvisits[3]=nk&0xff;
    includedb__fileWrite(&instance->file, keysNvisits, 4, 16);
    
    // todo remove this, we already have a seed
 //   const unsigned sd = instance->seed;
   // buf[0]=sd>>24; buf[1]=(sd>>16)&0xff; buf[2]=(sd>>8)&0xff; buf[3]=sd&0xff;
  //  includedb__fileWrite(&instance->file, buf, 4, 20);
    
    includedb__free(instance->buffer);
    includedb__fileClose(&instance->file);
    includedb__free(instance->occupied.bitvec);
    // todo: write all visits from nodes to file!
    includedb__free(instance->nodeVec);
    includedb__free(instance);
}

static const char *includedb_getError(includeDB *instance)
{
    if (!instance)
        return includedb__errorMsg;
    switch (instance->ec)
    {
        case INCLUDEDB__BITVEC_ALLOC:
            includedb__errorMsg = "Couldn't grow bitvec. Out of mem?";
            break;
        case INCLUDEDB__NODE_ALLOC:
            includedb__errorMsg = "Couldn't grow nodeVec. Out of mem?";
            break;
        case INCLUDEDB__BUFFER_ALLOC:
            includedb__errorMsg = "Failed to alloc bigger buffer";
            break;
        case INCLUDEDB__ALREADY_KEY:
            includedb__errorMsg = "Key already exists";
            break;
        case INCLUDEDB__SLOTS_ALLOC:
            includedb__errorMsg = "Couldn't allocate more slots. Out of mem?";
            break;
        default:
            includedb__errorMsg = "Ok";
    }
    return includedb__errorMsg;
}


/*++++++++++++++++++++++++++++++++++++++*/
/*                                Tests */
/*++++++++++++++++++++++++++++++++++++++*/
#if !defined(DISABLE_TESTS)


#endif // !defined(DISABLE_TESTS)



#endif // INCLUDEDB_IMPLEMENTATION


#endif // INCLUDEDB_DB_H
/*
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

