/*
  You need to add '#define NANOPULSE_DB_IMPLEMENTATION' before including this header in ONE source file.
  Like this:
      #define NANOPULSE_DB_IMPLEMENTATION
      #include "nanopulsedb.h"
 
  To disable compile-time unit tests:
      #define DISABLE_TESTS
 
  AUTHOR
     Professor Peanut
 
  LICENSE
      See end of file.
 
  HISTORY
       While in alpha, cross version compatibility is not guaranteed. Do not replace
       this file if you need to keep your database readable.
  
       0.1.0  Initial release
 
  PEANUTS
      BTC:  1H1RrCrEgUXDFibpaJciLjS9r7upQs6XPc
      BCH:  qzgfgd6zen70mfzasjtc4rx9m7fhz65zyg0n6v3sdh
      BSV:  15dtAGzzMf6yWF82aYuGKZYMCyP5HoWVLP
      ETH:  0x32a42d02eB021914FE8928d4A60332970F96f2cd
      DCR:  DsWY2Z1NThKqumM6x9oiyM3f2RkW28ruoyA
      LTC:  LWZ5HCcpModc1XcFpjEzz25J58eeQ8fJ7F
      DASH: XqMBmnxrgJWsvF7Hu3uBQ53TpcKLEsxsEi
      Thank you in advance!
      
*/

#ifndef NANOPULSE_DB_H
#define NANOPULSE_DB_H

#define NANOPULSE_VERSION_MAJOR 0
#define NANOPULSE_VERSION_MINOR 1
#define NANOPULSE_VERSION_PATCH 0
#define NANOPULSE_VERSION_NUM ((NANOPULSE_VERSION_MAJOR<<24) | \
                               (NANOPULSE_VERSION_MINOR<<16) | \
                               (NANOPULSE_VERSION_PATCH<< 8))

#if defined(__cplusplus)
  #define COMPTIME constexpr
  #define CTOR3(x,y,z) constexpr x,y,z
  #define CTOR4(w,x,y,z) constexpr w,x,y,z
#else
  #define COMPTIME static const
  #define constexpr
  #define nullptr ((void *)0)
  enum {false=0,true=1};
  typedef int bool;
  #define CTOR3(x,y,z)
  #define CTOR4(w,x,y,z)
  #ifndef DISABLE_TESTS
    #define DISABLE_TESTS
  #endif
#endif


struct nplse__bitvec;

typedef int (*pnplse__bitvecAlloc)(struct nplse__bitvec *bitvec, int amount);

typedef struct nplse__bitvec
{
    CTOR3(nplse__bitvec() : bitvec(nullptr), szVec(0), nplse__bitvecAlloc(nullptr) {} )
    unsigned *bitvec;
    int szVec;
    pnplse__bitvecAlloc nplse__bitvecAlloc;
} nplse__bitvec;


typedef struct nplse__skipnode
{
    CTOR4(nplse__skipnode() : nodeid(0), filepos(0), next(0), visits(0) {})
    unsigned nodeid;
    int filepos;
    int next; // pointer into 'nodeVec'
    int visits;
} nplse__skipnode;


struct nplse__file;

bool nplse__fileOpen(struct nplse__file *file, const char *filename);
bool nplse__fileCreate(struct nplse__file *file, const char *filename);
bool nplse__fileClose(struct nplse__file *file);
int  nplse__fileWrite(struct nplse__file *file, const unsigned char *bytes, int len, int filepos);
bool nplse__fileGrow(struct nplse__file *file, int len);

#define NANOPULSE_USE_STD_FILE_OPS

#if defined(NANOPULSE_USE_STD_FILE_OPS)
  #include <stdio.h>
  typedef struct nplse__file
  {
      FILE *pFile;
  } nplse__file;
  bool nplse__fileOpen(nplse__file *file, const char *filename)
  {
      file->pFile = fopen(filename, "rb+");
      return !!file->pFile;
  }
  bool nplse__fileCreate(nplse__file *file, const char *filename)
  {
      file->pFile = fopen(filename, "wb+");
      return !!file->pFile;
  }
  bool nplse__fileClose(nplse__file *file)
  {
      return fclose(file->pFile);
  }
  int  nplse__fileWrite(struct nplse__file *file, const unsigned char *bytes, int len, int filepos)
  {
      fseek(file->pFile, filepos, SEEK_SET);
      return fwrite(bytes, sizeof(unsigned char), len, file->pFile);
  }
  bool nplse__fileGrow(nplse__file *file, int len)
  {
      (void)file;
      (void)bytes;
      return true; // ¯\_(ツ)_/¯
  }
#endif

#if !defined(NANOPULSE_CHUNK_SIZE)
  #define NANOPULSE_CHUNK_SIZE 256
#endif

#if !defined(NANOPULSE_CACHE_SIZE)
  #define NANOPULSE_CACHE_SIZE (7*NANOPULSE_CHUNK_SIZE)
#endif

struct nanopulseDB;

typedef int (*pnplse__write)(struct nanopulseDB *instance, int location, int amount);
typedef void (*pnplse__read)(struct nanopulseDB *instance, int location, int amount);

enum nplse__errorCodes
{
    OK, BITVEC_ALLOC, SLOTS_ALLOC, BUFFER_ALLOC
};

typedef struct nanopulseDB
{
    // Buffer:
    unsigned char *buffer;
    int szBuf;
    
    // File:
    union
    {
        nplse__file file;
        unsigned char *mappedArray;
    };
    int chunkSize;
    pnplse__write nplse__write;
    pnplse__read nplse__read;
    nplse__bitvec occupied;
    
    // List:
    nplse__skipnode *nodeVec;
    int headA, headB, headC, headD;
    int nKeys;
    //int nAllocatedSlots = 32;
    //int addressNewNode = 0;
    int totalVisits;
    
    // Cache
    // cachesize // max # of entries in cache = sz/chunksize - - can be less than that
    
    // Hashing:
    unsigned seed;
    
    // Error code
    enum nplse__errorCodes ec;
} nanopulseDB;

// Error:
static const char *nplse__error;




/*
 ------------------------------------------------------------------------------
    Public interface
 ------------------------------------------------------------------------------
*/

// Add a new record
static constexpr int nplse_put(nanopulseDB *instance, const unsigned char *key, int keylen, const unsigned char *val, int vallen);

// Get a pointer to an existing record. *vallen contains the size of the returned value in bytes and can be NULL if
// you are not interested in it. Calling this is going to invalidate previously retuned pointers
static constexpr unsigned char *nplse_get(nanopulseDB *instance, unsigned char *key, int keylen, int *vallen);

// Delete record at key (TODO)
//static constexpr void nplse_delete(nanopulseDB *instance, unsigned char *key, int keylen);

// Upon opening a database a cursor is pointing to the first record. Use this to move the cursor to the next record
static constexpr void nplse_next(nanopulseDB *instance);

// Get the key at cursor position. Optionally the size of the key can be stored in *keylen. Pass NULL if this is not needed
static constexpr unsigned char *nplse_curGet(nanopulseDB *instance, int *keylen);

// Open existing, or create new
static nanopulseDB *nplse_open(const char *filename);

// Close. Must be called to ensure all changes are written to disk
static void nplse_close(nanopulseDB *instance);

// Get error
static const char *getError();








#ifdef NANOPULSE_DB_IMPLEMENTATION

#ifndef NANOPULSE_MALLOC
  #include <stdlib.h>
  #define nplse__malloc(sz)          malloc(sz)
  #define nplse__realloc(p,newsz)    realloc(p,newsz)
  #define nplse__free(p)             free(p)
#endif

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

static int nplse__bitvecAlloc(nplse__bitvec *bitvec, int amount)
{
    bitvec->szVec += amount; // alloc 32bit! sizeof(unsigned)
    bitvec->bitvec = (unsigned *)nplse__realloc(bitvec->bitvec, bitvec->szVec * sizeof(unsigned));
    if (bitvec->bitvec)
        return 0;
    return 1; // error
}

inline constexpr unsigned nplse__bitvecCheck(const nplse__bitvec *bitvec, int pos)
{
    const unsigned bit = bitvec->bitvec[pos>>5];
    return (bit >> (pos&31)) & 1;
}

inline constexpr void nplse__bitvecSet(nplse__bitvec *bitvec, int pos)
{
    const unsigned bit = 1 << (pos&31);
    bitvec->bitvec[pos>>5] |= bit;
}

inline constexpr int nplse__gatherSlots(nanopulseDB *instance, int requiredSlots)
{
    bool haveAvail = false;
    int location = 0;
    nplse__bitvec *bitvec = &instance->occupied;
    for (; location<((bitvec->szVec*32)-requiredSlots) && !haveAvail; ++location)
        if (nplse__bitvecCheck(bitvec, location) == 0)
        {
            haveAvail = true;
            for (int i=1; i<requiredSlots-1; ++i)
                haveAvail = haveAvail && (nplse__bitvecCheck(bitvec, location+i) == 0);
        }
    location -= 1;
    if (!haveAvail)
    {
        // make room:
        const int amount = (requiredSlots/32) + 1;
        if (bitvec->nplse__bitvecAlloc(bitvec, amount))
        {
            COMPTIME char error[] = "Couldn't grow bitvec. Out of mem?";
            instance->ec = BITVEC_ALLOC;
            return -1; // failed
        }
        // todo : grow file
        return nplse__gatherSlots(instance, requiredSlots);
    }
    return location;
}

inline constexpr void nplse__markSlots(nplse__bitvec *bitvec, int start, int len)
{
    for (int i=0; i<len; i++)
        nplse__bitvecSet(bitvec, start+i);
}

inline constexpr int nplse__getNewKeyPos(nanopulseDB *instance)
{
    return instance->nKeys++;
}

inline constexpr void nplse__insertSkipnode(nanopulseDB *instance, unsigned key, int filepos)
{
    const int newNodeAddr = nplse__getNewKeyPos(instance);
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

static int nplse__dbWrite(struct nanopulseDB *instance, int location, int amount)
{
    location += 256; // db header size
    const int res = nplse__fileWrite(&instance->file, instance->buffer, amount, location);
    return !(res==amount);
}

//static void nplse__dbRead(struct nanopulseDB *instance, int location, int amount)
//{
//    location += 256;
//    //nplse__fileRead(&instance->file, );
//}

COMPTIME int nplse__header_keyhashLen        = 4;
COMPTIME int nplse__header_keylenLen         = 4;
COMPTIME int nplse__header_vallenLen         = 4;
COMPTIME int nplse__header_recordPriorityLen = 4;












static constexpr int nplse_put(nanopulseDB *instance, const unsigned char *key, int keylen, const unsigned char *val, int vallen)
{
    return -1;
}

static nanopulseDB *nplse_open(const char *filename)
{
    COMPTIME unsigned char magic[] = "npdb";
    COMPTIME unsigned v = NANOPULSE_VERSION_NUM;
    COMPTIME unsigned char versionStr[] = {(v>>24)&0xff, (v>>16)&0xff, (v>>8)&0xff, v&0xff};
    nanopulseDB *newInstance = (nanopulseDB *)nplse__malloc(sizeof(nanopulseDB));
    if (!newInstance)
    {
        COMPTIME char error[] = "Couldn't create db. Out of mem?";
        return nullptr;
    }
    
    int nKeys = 0;
    if (!nplse__fileOpen(&newInstance->file, filename))
    {
        if (!nplse__fileCreate(&newInstance->file, filename))
        {
            COMPTIME char error[] = "Couldn't open db. File corrupted?";
            // Cannot call close() on an instance that hasn't been created correctly.
            // Return NULL instead:
            nplse__free(newInstance);
            return nullptr;
        }
        // write signature
        nplse__fileWrite(&newInstance->file, magic, 4, 0);
        // write version
        nplse__fileWrite(&newInstance->file, versionStr, 4, 4);
        // write chunksize
        COMPTIME unsigned cs = NANOPULSE_CHUNK_SIZE;
        COMPTIME unsigned char chunksize[] = {(cs>>24)&0xff, (cs>>16)&0xff, (cs>>8)&0xff, cs&0xff};
        nplse__fileWrite(&newInstance->file, chunksize, 4, 8);
        newInstance->chunkSize = cs;
        // write keys & "visits"
        COMPTIME unsigned char keysNvisits[] = {0,0,0,0,0,0,0,0};
        nplse__fileWrite(&newInstance->file, keysNvisits, 8, 12);
        newInstance->totalVisits = 0;
        // create seed
        const unsigned sd = 6969; // todo
        const unsigned char seedStr[] = {(sd>>24)&0xff, (sd>>16)&0xff, (sd>>8)&0xff, sd&0xff};
        nplse__fileWrite(&newInstance->file, seedStr, 4, 20);
        newInstance->seed = sd;
    }
    else
    {
        // read npdb
        unsigned char buf[24];
        nplse__fileRead(&newInstance->file, buf, 20, 0);
        bool ok = buf[0]=='n' && buf[1]=='p' && buf[2]=='d' && buf[3]=='b';
        if (!ok)
        {
            COMPTIME char error[] = "Couldn't open db. File not recognized";
            nplse__free(newInstance);
            return nullptr;
        }
        // read version vs minimum required
        ok =       buf[4] == NANOPULSE_VERSION_MAJOR;
        ok = ok && buf[5] <= NANOPULSE_VERSION_MINOR;
        ok = ok && buf[6] <= NANOPULSE_VERSION_PATCH;
        (void)     buf[7];
        if (!ok)
        {
            COMPTIME char error[] = "Couldn't open db. Incompatible version";
            nplse__free(newInstance);
            return nullptr;
        }
        // read chunksise
        newInstance->chunkSize = (buf[8]<<24) | (buf[9]<<16) | (buf[10]<<8) | buf[11];
        // read keys
        nKeys = (buf[12]<<24) | (buf[13]<<16) | (buf[14]<<8) | buf[15];
        // read visits
          // todo: not yet
        // read randseed
        newInstance->seed = (buf[20]<<24) | (buf[21]<<16) | (buf[22]<<8) | buf[23];
        printf("nkeys %d \n", newInstance->seed);
            
            
        // chunkSize =
        // read filesz
        // read vists
        
        // if # of total acces > some 'limit', /2!!
    }
    // Create buffer:
    newInstance->buffer = (unsigned char *)nplse__malloc(256);
    newInstance->szBuf = 1;
    // Write fn:
    newInstance->nplse__write = nplse__dbWrite;
    // Read fn:
    newInstance->nplse__read = nullptr;
    // setup bitvec:
    newInstance->occupied.bitvec = (unsigned *)nplse__malloc(sizeof(unsigned) * 1);
    newInstance->occupied.szVec = 1;
    newInstance->occupied.nplse__bitvecAlloc = nplse__bitvecAlloc;
    // setup node list:
    newInstance->nodeVec = (nplse__skipnode *)nplse__malloc(sizeof(nplse__skipnode) * 32 * 4);
    
    // Build index:
    COMPTIME int dbHeaderSize = 256;
    int offset = dbHeaderSize;
    unsigned char buf[20];
    for (int i=0; i<nKeys; ++i)
    {
        nplse__fileRead(&newInstance->file, buf, 16, offset);
        const unsigned keyhash = (buf[ 0]<<24) | (buf[ 1]<<16) | (buf[ 2]<<8) | buf[ 3];
        nplse__insertSkipnode(newInstance, keyhash, offset);
        // get position for the next record:
        const unsigned keylen = (buf[ 4]<<24) | (buf[ 5]<<16) | (buf[ 6]<<8) | buf[ 7];
        const unsigned vallen = (buf[ 8]<<24) | (buf[ 9]<<16) | (buf[10]<<8) | buf[11];
        const int requiredSizeInByte = nplse__header_keyhashLen
                                     + nplse__header_keylenLen
                                     + nplse__header_vallenLen
                                     + nplse__header_recordPriorityLen
                                     + keylen
                                     + vallen
                                     + 1;
        offset += (requiredSizeInByte / newInstance->chunkSize) + 1;
    }

    return newInstance;
}

static void nplse_close(nanopulseDB *instance)
{
    if (instance == nullptr)
        return;
        
    unsigned char buf[4];
    const int nk = instance->nKeys;
    buf[0]=nk>>24; buf[1]=(nk>>16)&0xff; buf[2]=(nk>>8)&0xff; buf[3]=nk&0xff;
    nplse__fileWrite(&instance->file, buf, 4, 12);
    
    const unsigned sd = instance->seed;
    buf[0]=sd>>24; buf[1]=(sd>>16)&0xff; buf[2]=(sd>>8)&0xff; buf[3]=sd&0xff;
    nplse__fileWrite(&instance->file, buf, 4, 20);
    
    
    // todo save cache
    nplse__free(instance->buffer);
    nplse__fileClose(&instance->file);
    nplse__free(instance->occupied.bitvec);
    // todo: write all visits from nodes to file!
    nplse__free(instance->nodeVec);
    nplse__free(instance);
}












#if !defined(DISABLE_TESTS)

constexpr unsigned nplse__testCache()
{
    return 0;
}
static constexpr unsigned resCache = nplse__testCache();


constexpr unsigned nplse__testBitvec()
{
    unsigned bitvecBits[3] = {0};
    nplse__bitvec testBitvec;
    testBitvec.bitvec = bitvecBits;
    for (int i=0; i<64; ++i)
        nplse__bitvecSet(&testBitvec, i);
    const bool TEST_BIT_63 =  nplse__bitvecCheck(&testBitvec, 63)
                           && !nplse__bitvecCheck(&testBitvec, 64);
    nplse__bitvecSet(&testBitvec, 64);
    const bool TEST_BIT_64 =  nplse__bitvecCheck(&testBitvec, 64)
                           && !nplse__bitvecCheck(&testBitvec, 65);
    
    return  TEST_BIT_63
         | (TEST_BIT_64 << 1);
}
static constexpr unsigned resBitvec = nplse__testBitvec();

static_assert(resBitvec&1, "bit 63 not correct");
static_assert(resBitvec&2, "bit 64 not correct");


constexpr unsigned nplse__testSlots()
{
    nanopulseDB testDB{ .mappedArray=nullptr,
                        .chunkSize=0,
                        .nodeVec=nullptr,
                        .seed=0
                      };
    unsigned bitvecBits[3/* *32 */] = {0};
    testDB.occupied.bitvec = bitvecBits;
    testDB.occupied.szVec  = 1;
    auto testAlloc = [](nplse__bitvec *bitvec, int amount)->int
                     {
                         if ((bitvec->szVec+amount) <= 3)
                         {
                             bitvec->szVec += amount;
                             return 0;
                         }
                         return 1;
                     };
    testDB.occupied.nplse__bitvecAlloc = testAlloc;
    const bool TEST_HAVE_ZERO_SLOTS = nplse__gatherSlots(&testDB, 0) == 0;
    int location = nplse__gatherSlots(&testDB, 2);
    nplse__markSlots(&testDB.occupied, location, 2);
    const bool TEST_TWO_SLOTS_OCCUPIED =  (bitvecBits[0]&1)
                                       && (bitvecBits[0]&2)
                                       && !(bitvecBits[0]&4)
                                       && location==0;
    location = nplse__gatherSlots(&testDB, 1);
    nplse__markSlots(&testDB.occupied, location, 1);
    const bool TEST_LOCATION_TWO = location == 2;
    location = nplse__gatherSlots(&testDB, 7);
    nplse__markSlots(&testDB.occupied, location, 7);
    const bool TEST_LOCATION_THREE = location == 3;
    // test large allocation:
    location = nplse__gatherSlots(&testDB, 24+39);
    nplse__markSlots(&testDB.occupied, location, 24+39);
    const bool TEST_LOCATION_EIGHT = location == 10;
    location = nplse__gatherSlots(&testDB, 22);
    nplse__markSlots(&testDB.occupied, location, 22);
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
static constexpr unsigned resSlots = nplse__testSlots();

static_assert(resSlots&    1, "couldn't gather 0 slots");
static_assert(resSlots&    2, "slots were not correctly marked as 'occupied'");
static_assert(resSlots&    4, "incorrect slot position (2)");
static_assert(resSlots&    8, "incorrect slot position (3)");
static_assert(resSlots&   16, "incorrect slot position (8)");
static_assert(resSlots&   32, "incorrect slot position (8+24+39)");
static_assert(resSlots&   64, "flags set incorrectly");


constexpr unsigned nplse__testSkiplist()
{
    // setup:
    nplse__skipnode testNodes[32];
    nanopulseDB testDB{ .mappedArray=nullptr,
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

#endif // !defined(DISABLE_TESTS)

#endif // NANOPULSE_DB_IMPLEMENTATION


#endif // NANOPULSE_DB_H

/*
 ------------------------------------------------------------------------------
 Copyright (c) 2020 Professor Peanut
 
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
