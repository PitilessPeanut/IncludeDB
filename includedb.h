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

#ifndef INCLUDEDB_DB_H
#define INCLUDEDB_DB_H

#define INCLUDEDB_VERSION_MAJOR 0
#define INCLUDEDB_VERSION_MINOR 1
#define INCLUDEDB_VERSION_PATCH 0
#define INCLUDEDB_VERSION_NUM ((INCLUDEDB_VERSION_MAJOR<<24) | \
                               (INCLUDEDB_VERSION_MINOR<<16) | \
                               (INCLUDEDB_VERSION_PATCH<< 8))

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


// typedef size_t icldb__size;


struct icldb__bitvec;

typedef int (*pIcldb__bitvecAlloc)(struct icldb__bitvec *bitvec, int amount);

typedef struct icldb__bitvec
{
    CTOR3(icldb__bitvec() : bitvec(nullptr), szVecIn32Chunks(0), icldb__bitvecAlloc(nullptr) {})
    unsigned *bitvec;
    int szVecIn32Chunks;
    pIcldb__bitvecAlloc icldb__bitvecAlloc;
} icldb__bitvec;


typedef struct icldb__skipnode
{
    CTOR4(icldb__skipnode() : nodeid(0), filepos(0), next(0), visits(0) {})
    unsigned nodeid;
    int filepos;
    int next; // pointer into 'nodeVec'
    int visits;
} icldb__skipnode;


struct icldb__file;

bool icldb__fileOpen(struct icldb__file *file, const char *filename);
bool icldb__fileCreate(struct icldb__file *file, const char *filename);
bool icldb__fileClose(struct icldb__file *file);
int  icldb__fileWrite(struct icldb__file *file, const unsigned char *bytes, int len, int filepos);
void icldb__fileRead(struct icldb__file *file, unsigned char *bytes, int len, int filepos);
bool icldb__fileGrow(struct icldb__file *file, int len);

#if defined(_WIN32) && !defined(INCLUDEDB_USE_STD_FILE_OPS)
  #include <winbase.h>
  #define INCLUDEDB_USE_STD_FILE_OPS
#else
  #include <sys/mman.h>
  #define INCLUDEDB_USE_STD_FILE_OPS
#endif

#if defined(INCLUDEDB_USE_STD_FILE_OPS)
  #include <stdio.h>
  typedef struct icldb__file
  {
      FILE *pFile;
  } icldb__file;

  bool icldb__fileOpen(icldb__file *file, const char *filename)
  {
      file->pFile = fopen(filename, "rb+");
      return !!file->pFile;
  }

  bool icldb__fileCreate(icldb__file *file, const char *filename)
  {
      file->pFile = fopen(filename, "wb+");
      return !!file->pFile;
  }

  bool icldb__fileClose(icldb__file *file)
  {
      return fclose(file->pFile);
  }

  int  icldb__fileWrite(icldb__file *file, const unsigned char *bytes, int len, int filepos)
  {
      fseek(file->pFile, filepos, SEEK_SET);
      return fwrite(bytes, sizeof(unsigned char), len, file->pFile);
  }

  void icldb__fileRead(icldb__file *file, unsigned char *bytes, int len, int filepos)
  {
      fseek(file->pFile, filepos, SEEK_SET);
      const int r = fread(bytes, sizeof(unsigned char), len, file->pFile);
      (void)r;
  }

  bool icldb__fileGrow(icldb__file *file, int len)
  {
      // ¯\_(ツ)_/¯
      (void)file;
      (void)len;
      return true;
  }
#else

#endif

#if !defined(INCLUDEDB_CHUNK_SIZE)
  #define INCLUDEDB_CHUNK_SIZE 256
#endif

struct includeDB;

typedef int (*pIcldb__write)(struct includeDB *instance, int location, int amount);
typedef int (*pIcldb__read)(struct includeDB *instance, int location, int amount);

typedef int (*pIcldb__nodevecAlloc)(struct includeDB *instance, int newSize);

COMPTIME int szBloommap = sizeof(unsigned) * 8;

enum icldb__errorCodes
{
    ICLDB__OK,
    ICLDB__BITVEC_ALLOC,
    ICLDB__NODE_ALLOC,
    ICLDB__SLOTS_ALLOC,
    ICLDB__BUFFER_ALLOC,
    ICLDB__ALREADY_KEY,
    ICLDB__KEY_NOT_FOUND
};

typedef struct includeDB
{
    // Buffer/Address:
    union
    {
        unsigned char *buffer;
        unsigned char *address;
    };
    int szBuf;
    
    // File:
    union
    {
        icldb__file file;
        unsigned char *mappedArray;
    };
    int chunkSize;
    pIcldb__write icldb__write;
    pIcldb__read icldb__read;
    icldb__bitvec occupied;
    
    // List:
    icldb__skipnode *nodeVec;
    int headA, headB, headC, headD;
    int nKeys;
    int nAllocated;
    pIcldb__nodevecAlloc icldb__nodevecAlloc;
    //int addressNewNode = 0; // todo
    int globalVisits;
    int cursor;
    
    // Hashing:
    unsigned seed;
    unsigned bloommap;
    int bloomcounters[szBloommap];
    
    // Error code
    enum icldb__errorCodes ec;
} includeDB;

// Error:
static const char *icldb__errorMsg;




/*
 ------------------------------------------------------------------------------
    Public interface
 ------------------------------------------------------------------------------
*/

// Add a new record
static constexpr int icldb_put(includeDB *instance, const unsigned char *key, int keylen, const unsigned char *val, int vallen);

// Get a pointer to an existing record. *vallen contains the size of the returned value in bytes and can be NULL if
// you are not interested in it. Calling this is going to invalidate previously retuned pointers
static constexpr unsigned char *icldb_get(includeDB *instance, const unsigned char *key, int keylen, int *vallen);

// Delete record at key (TODO)
//static constexpr void icldb_delete(includeDB *instance, unsigned char *key, int keylen);

// Upon opening a database a cursor is pointing to the first record. Use this to move the cursor to the next record
static constexpr void icldb_next(includeDB *instance);

// Get the key at cursor position. Optionally the size of the key can be stored in *keylen. Pass NULL if this (<-todo what?) is not needed
static constexpr unsigned char *icldb_curGet(includeDB *instance, int *keylen);

// Open existing, or create new
static includeDB *icldb_open(const char *filename);

// Close. Must be called to ensure all changes are written to disk
static void icldb_close(includeDB *instance);

// Reset all key priorities
static void icldb_resetPriorities(includeDB *instance);

// Get error
static const char *icldb_getError(includeDB *instance);








#ifdef INCLUDEDB_IMPLEMENTATION

#ifndef INCLUDEDB_MALLOC
  #include <stdlib.h>
  #define icldb__malloc(sz)          malloc(sz)
  #define icldb__realloc(p,newsz)    realloc(p,newsz)
  #define icldb__zeroalloc(cnt,sz)   calloc(cnt,sz)
  #define icldb__free(p)             free(p)
#endif

#ifndef INCLUDEDB_TIME
  #include <time.h>
#endif

inline constexpr void icldb__bloomPut(includeDB *instance)
{
    (void)instance;
}

inline constexpr void icldb__bloomRemove(includeDB *instance)
{
    (void)instance;
}

inline constexpr bool icldb__bloomMaybeHave(const includeDB *instance, const unsigned hash)
{
    const unsigned h = hash & 0xffffffff;
    return true; // !((h & instance->bloommap) ^ h);
}

static constexpr unsigned icldb__xx32(const unsigned char *input, int len, unsigned seed)
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

static int icldb__bitvecAlloc(icldb__bitvec *bitvec, int amount)
{
    const int szOld = bitvec->szVecIn32Chunks;
    bitvec->szVecIn32Chunks += amount;
    unsigned *tmp = (unsigned *)icldb__zeroalloc(bitvec->szVecIn32Chunks, sizeof(unsigned));
    if (!tmp)
        return 1; // 1 is error
    for (int i=0; i<szOld; ++i)
        tmp[i] = bitvec->bitvec[i];
    icldb__free(bitvec->bitvec);
    bitvec->bitvec = tmp;
    return 0;
}

inline constexpr unsigned icldb__bitvecCheck(const icldb__bitvec *bitvec, int pos)
{
    const unsigned bit = bitvec->bitvec[pos>>5];
    return (bit >> (pos&31)) & 1;
}

inline constexpr void icldb__bitvecSet(icldb__bitvec *bitvec, int pos)
{
    const unsigned bit = 1 << (pos&31);
    bitvec->bitvec[pos>>5] |= bit;
}

inline constexpr int icldb__gatherSlots(includeDB *instance, int requiredSlots)
{
    bool haveAvail = false;
    int location = 0;
    icldb__bitvec *bitvec = &instance->occupied;
    for (; location<((bitvec->szVecIn32Chunks*32)-requiredSlots) && !haveAvail; ++location)
        if (icldb__bitvecCheck(bitvec, location) == 0)
        {
            haveAvail = true;
            for (int i=1; i<requiredSlots-1; ++i)
                haveAvail = haveAvail && (icldb__bitvecCheck(bitvec, location+i) == 0);
        }
    location -= 1;
    if (!haveAvail)
    {
        // make room:
        const int amount = (requiredSlots/32) + 1;
        if (bitvec->icldb__bitvecAlloc(bitvec, amount))
        {
            instance->ec = ICLDB__BITVEC_ALLOC;
            return -1; // failed
        }
        // todo : grow file 32*chunksz
        return icldb__gatherSlots(instance, requiredSlots);
    }
    return location;
}

inline constexpr void icldb__markSlots(icldb__bitvec *bitvec, int start, int len)
{
    for (int i=0; i<len; i++)
        icldb__bitvecSet(bitvec, start+i);
}

static int icldb__nodevecAlloc(includeDB *instance, int newSize)
{
    icldb__skipnode *tmp = (icldb__skipnode *)icldb__realloc(instance->nodeVec, newSize * sizeof(icldb__skipnode));
    if (!tmp)
        return 1; // error
    instance->nodeVec = tmp;
    return 0;
}

static constexpr int icldb__getNewKeyPos(includeDB *instance)
{
    if (instance->nKeys == instance->nAllocated)
    {
        const int newSize = instance->nAllocated << 1;
        if (instance->icldb__nodevecAlloc(instance, newSize) == 1)
        {
            instance->ec = ICLDB__NODE_ALLOC;
            return instance->nKeys;
        }
        instance->nAllocated = newSize;
    }
    return instance->nKeys++;
}

static constexpr void icldb__insertSkipnode(includeDB *instance, unsigned key, int pos, int layer)
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

static constexpr void icldb__insertNewSkipnode(includeDB *instance, unsigned key, int filepos)
{
    const int newNodeAddr = icldb__getNewKeyPos(instance);
    instance->nodeVec[newNodeAddr].nodeid  = key;
    instance->nodeVec[newNodeAddr].filepos = filepos;
    
    icldb__insertSkipnode(instance, key, newNodeAddr, 3);
}

static constexpr int icldb__findPrevSkipnode(includeDB *instance, const unsigned key, const int start, const int layer)
{
    int current = start;
    int prev = current;
    for (int i=0; i<instance->nKeys; ++i)
    {
        if (layer<3 && (prev==current || instance->nodeVec[current].nodeid>key))
            return icldb__findPrevSkipnode(instance, key, instance->nodeVec[current].next, layer+1);
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

inline constexpr icldb__skipnode *icldb__findSkipnode(includeDB *instance, const unsigned key)
{
    //const int res = icldb__findPrevSkipnode(instance, key, instance->headA, 0);
    const int res = icldb__findPrevSkipnode(instance, key, instance->headD, 3);
    return instance->nodeVec[res].nodeid == key ? &instance->nodeVec[res] : nullptr;
}

static constexpr int icldb__dbBufferResize(includeDB *instance, int newsize)
{
    const int sz = instance->szBuf;
    if (sz < newsize)
    {
        instance->szBuf = ((newsize/sz) * sz) + sz;
        unsigned char *tmp = (unsigned char *)icldb__realloc(instance->buffer, instance->szBuf);
        if (!tmp)
        {
            instance->ec = ICLDB__BUFFER_ALLOC;
            return 1;
        }
        instance->buffer = tmp;
    }
    return 0; // Ok
}

static int icldb__dbWrite(includeDB *instance, int location, int amount)
{
    location += 128; // db header size
    const int res = icldb__fileWrite(&instance->file, instance->buffer, amount, location);
    return res != amount;
}

static int icldb__dbRead(includeDB *instance, int location, int amount)
{
    location += 128;
    
    // make sure the buffer is big enough:
    if (icldb__dbBufferResize(instance, amount) == 1)
        return 1; // Error
    
    icldb__fileRead(&instance->file, instance->buffer, amount, location);
    return 0;
}

static constexpr int icldb__dbAddressResize(includeDB *instance, int newsize)
{
    (void)instance;
    (void)newsize;
    return 0; // Ok
}

static int icldb__dbAddressWrite(includeDB *instance, int location, int amount)
{
    //todo
    (void)instance;
    (void)location;
    (void)amount;
    return 0;
}

static int icldb__dbAddressRead(includeDB *instance, int location, int amount)
{
    // todo
    (void)instance;
    (void)location;
    (void)amount;
    return 0;
}

COMPTIME int icldb__header_keyhashLen        = 4;
COMPTIME int icldb__header_keylenLen         = 4;
COMPTIME int icldb__header_vallenLen         = 4;
COMPTIME int icldb__header_recordPriorityLen = 4;




/*
 ------------------------------------------------------------------------------
    Public functions
 ------------------------------------------------------------------------------
*/

static constexpr int icldb_put(includeDB *instance, const unsigned char *key, int keylen, const unsigned char *val, int vallen)
{
    const unsigned keyhash = icldb__xx32(key, keylen, instance->seed);
    if (icldb_get(instance, key, keylen, nullptr))
    {
        instance->ec = ICLDB__ALREADY_KEY;
        return 1;
    };
    const int chunkSize = instance->chunkSize;
    const int requiredSizeInByte = icldb__header_keyhashLen
                                 + icldb__header_keylenLen
                                 + icldb__header_vallenLen
                                 + icldb__header_recordPriorityLen
                                 + keylen
                                 + vallen
                                 + 1;
    const int requiredSlots = (requiredSizeInByte/chunkSize) + 1; // !!(requiredSizeInByte%chunkSize);
    const int requiredSizeOfRecord = requiredSlots*chunkSize;
    const int location = icldb__gatherSlots(instance, requiredSlots);
    if (location == -1)
    {
        instance->ec = ICLDB__SLOTS_ALLOC;
        return 1;
    }
    icldb__markSlots(&instance->occupied, location, requiredSlots);
    // Hook up new node:
    icldb__insertNewSkipnode(instance, keyhash, location*chunkSize);
    // Write data:
    if (icldb__dbBufferResize(instance, requiredSizeOfRecord))
        return 1; // Error
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
    return instance->icldb__write(instance, location*chunkSize, requiredSizeOfRecord);
}

static constexpr unsigned char *icldb_get(includeDB *instance, const unsigned char *key, int keylen, int *vallen)
{
    instance->ec = ICLDB__KEY_NOT_FOUND;
    const unsigned keyhash = icldb__xx32(key, keylen, instance->seed);
    if (icldb__bloomMaybeHave(instance, keyhash) == false)
        return nullptr;
        
    // todo increase priority
    
    COMPTIME unsigned headerSize = icldb__header_keyhashLen
                                 + icldb__header_keylenLen
                                 + icldb__header_vallenLen
                                 + icldb__header_recordPriorityLen;
    icldb__skipnode *found = icldb__findSkipnode(instance, keyhash);
    if (found)
    {
        // read header
        instance->icldb__read(instance, found->filepos, headerSize);
        unsigned char *data = &instance->buffer[icldb__header_keyhashLen];
        const unsigned kl = (data[0]<<24) | (data[1]<<16) | (data[2]<< 8) | data[3];
        data += icldb__header_keylenLen;
        const unsigned vl = (data[0]<<24) | (data[1]<<16) | (data[2]<< 8) | data[3];
        // read rest
        instance->icldb__read(instance, found->filepos+headerSize + kl, vl);
        int unusedVallen = 0; // In case vallen == nullptr
        vallen = vallen ? : &unusedVallen;
        *vallen = vl;
        instance->ec = ICLDB__OK;
        return instance->buffer;
    }
    return nullptr;
}

//static constexpr void icldb_delete(includeDB *instance, unsigned char *key, int keylen)
//{
//}

static constexpr void icldb_next(includeDB *instance)
{
    instance->cursor += 1;
}

static constexpr unsigned char *icldb_curGet(includeDB *instance, int *keylen)
{
    *keylen = 0;
    if (instance->cursor == instance->nKeys)
        return nullptr;
    COMPTIME int headerSize = icldb__header_keyhashLen
                            + icldb__header_keylenLen
                            + icldb__header_vallenLen
                            + icldb__header_recordPriorityLen;
    const int filepos = instance->nodeVec[instance->cursor].filepos;
    if (instance->icldb__read(instance, filepos, headerSize) != 0)
        return nullptr;
    const unsigned char *data = instance->buffer;
    const int kl = (data[4]<<24) | (data[5]<<16) | (data[6]<<8) | data[7];
    if (instance->icldb__read(instance, filepos+headerSize, kl) != 0)
        return nullptr;
    *keylen = kl;
    return instance->buffer;
}

static includeDB *icldb_open(const char *filename)
{
    COMPTIME unsigned char magic[] = "icld";
    COMPTIME unsigned v = INCLUDEDB_VERSION_NUM;
    COMPTIME unsigned char versionStr[] = {(v>>24)&0xff, (v>>16)&0xff, (v>>8)&0xff, v&0xff};
    includeDB *newInstance = (includeDB *)icldb__malloc(sizeof(includeDB));
    if (!newInstance)
    {
        icldb__errorMsg = "Couldn't alloc db. Out of mem?";
        return nullptr;
    }
    
    int nKeys = 0;
    if (!icldb__fileOpen(&newInstance->file, filename))
    {
        if (!icldb__fileCreate(&newInstance->file, filename))
        {
            icldb__errorMsg = "Couldn't open db. File corrupted?";
            // Cannot call close() on an instance that hasn't been created correctly.
            // Return NULL instead:
            icldb__free(newInstance);
            return nullptr;
        }
        // write signature
        icldb__fileWrite(&newInstance->file, magic, 4, 0);
        // write version
        icldb__fileWrite(&newInstance->file, versionStr, 4, 4);
        // write chunksize
        COMPTIME unsigned cs = INCLUDEDB_CHUNK_SIZE;
        COMPTIME unsigned char chunksize[] = {(cs>>24)&0xff, (cs>>16)&0xff, (cs>>8)&0xff, cs&0xff};
        icldb__fileWrite(&newInstance->file, chunksize, 4, 8);
        newInstance->chunkSize = cs;
        // write keys & "visits"
        COMPTIME unsigned char keysNvisits[] = {0,0,0,0,0,0,0,0};
        icldb__fileWrite(&newInstance->file, keysNvisits, 8, 12);
        newInstance->globalVisits = 0;
        // create seed
        const unsigned sd = 6969; // todo get from time
        const unsigned char seedStr[] = {(sd>>24)&0xff, (sd>>16)&0xff, (sd>>8)&0xff, sd&0xff};
        icldb__fileWrite(&newInstance->file, seedStr, 4, 20);
        newInstance->seed = sd;
    }
    else
    {
        // read icld
        unsigned char buf[24];
        icldb__fileRead(&newInstance->file, buf, 24, 0);
        bool ok = buf[0]=='i' && buf[1]=='c' && buf[2]=='l' && buf[3]=='d';
        if (!ok)
        {
            icldb__errorMsg = "Couldn't open db. File not recognized";
            icldb__free(newInstance);
            return nullptr;
        }
        // read version vs minimum required
        ok =       buf[4] == INCLUDEDB_VERSION_MAJOR;
        ok = ok && buf[5] <= INCLUDEDB_VERSION_MINOR;
        ok = ok && buf[6] <= INCLUDEDB_VERSION_PATCH;
        (void)     buf[7];
        if (!ok)
        {
            // todo upgrade version if possible!
            
            icldb__errorMsg = "Couldn't open db. Incompatible version";
            icldb__free(newInstance);
            return nullptr;
        }
        // read chunksise
        newInstance->chunkSize = (buf[8]<<24) | (buf[9]<<16) | (buf[10]<<8) | buf[11];
        // read # of keys
        nKeys = (buf[12]<<24) | (buf[13]<<16) | (buf[14]<<8) | buf[15];
        // read visits
          // todo: not yet
        // read randseed
        newInstance->seed = (buf[20]<<24) | (buf[21]<<16) | (buf[22]<<8) | buf[23];
        printf("seed %d  %d %d %d %d \n", newInstance->seed, buf[20],buf[21],buf[22],buf[23]);
        
        
        // chunkSize =
        // read filesz
        // read vists
        
        // if # of total acces > some 'limit', /2!!
    }
    // Create buffer:
    newInstance->buffer = (unsigned char *)icldb__malloc(newInstance->chunkSize);
    newInstance->szBuf = 1;
    // Write fn:
    newInstance->icldb__write = icldb__dbWrite;
    // Read fn:
    newInstance->icldb__read = icldb__dbRead;
    // setup bitvec:
    newInstance->occupied.bitvec = (unsigned *)icldb__zeroalloc(1, sizeof(unsigned));
    newInstance->occupied.szVecIn32Chunks = 1;
    newInstance->occupied.icldb__bitvecAlloc = icldb__bitvecAlloc;
    // setup node list:
    newInstance->nodeVec = (icldb__skipnode *)icldb__malloc(sizeof(icldb__skipnode) * sizeof(unsigned) * 8);
    newInstance->nAllocated = sizeof(unsigned) * 8;
    newInstance->icldb__nodevecAlloc = icldb__nodevecAlloc;
    
    // todo: init heads
    newInstance->headD = 0;
    
    
    newInstance->nKeys = 0;
    // put cursor to the start
    newInstance->cursor = 0;
    // init bloomfilter
    newInstance->bloommap = 0;
    for (int i=0; i<szBloommap; ++i)
        newInstance->bloomcounters[i] = 0;
    // reset error
    newInstance->ec = ICLDB__OK;
    
    
    printf("num ky %d \n", nKeys);
    
    // Build index:
    COMPTIME int dbHeaderSize = 128;
    int offset = 0;
    unsigned char buf[20];
    for (int i=0; i<nKeys; ++i)
    {
        icldb__fileRead(&newInstance->file, buf, 16, offset+dbHeaderSize);
        const unsigned keyhash = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
        icldb__insertNewSkipnode(newInstance, keyhash, offset);
        // get position for the next record:
        const unsigned keylen = (buf[ 4]<<24) | (buf[ 5]<<16) | (buf[ 6]<<8) | buf[ 7];
        const unsigned vallen = (buf[ 8]<<24) | (buf[ 9]<<16) | (buf[10]<<8) | buf[11];
        const int requiredSizeInByte = icldb__header_keyhashLen
                                     + icldb__header_keylenLen
                                     + icldb__header_vallenLen
                                     + icldb__header_recordPriorityLen
                                     + keylen
                                     + vallen
                                     + 1;
        const int requiredSlots = (requiredSizeInByte/newInstance->chunkSize) + 1;
        const int slotLocation = icldb__gatherSlots(newInstance, requiredSlots);
        icldb__markSlots(&newInstance->occupied, slotLocation, requiredSlots);
        offset += newInstance->chunkSize * requiredSlots;
    }
    
    return newInstance;
}

static void icldb_close(includeDB *instance)
{
    if (instance == nullptr)
        return;
        
    unsigned char buf[4];
    const int nk = instance->nKeys;
    buf[0]=nk>>24; buf[1]=(nk>>16)&0xff; buf[2]=(nk>>8)&0xff; buf[3]=nk&0xff;
    icldb__fileWrite(&instance->file, buf, 4, 12);
    
    // todo write inm.!!
    const unsigned sd = instance->seed;
    buf[0]=sd>>24; buf[1]=(sd>>16)&0xff; buf[2]=(sd>>8)&0xff; buf[3]=sd&0xff;
    icldb__fileWrite(&instance->file, buf, 4, 20);
    
    icldb__free(instance->buffer);
    icldb__fileClose(&instance->file);
    icldb__free(instance->occupied.bitvec);
    // todo: write all visits from nodes to file!
    icldb__free(instance->nodeVec);
    icldb__free(instance);
}

static void icldb_resetPriorities(includeDB *instance)
{
    // make sure 'tombstone' priorities are NOT reset!
}

static const char *icldb_getError(includeDB *instance)
{
    if (!instance)
        return icldb__errorMsg;
    switch (instance->ec)
    {
        case ICLDB__BITVEC_ALLOC:
            icldb__errorMsg = "Couldn't grow bitvec. Out of mem?";
            break;
        case ICLDB__NODE_ALLOC:
            icldb__errorMsg = "Couldn't grow nodeVec. Out of mem?";
            break;
        case ICLDB__BUFFER_ALLOC:
            icldb__errorMsg = "Failed to alloc bigger buffer";
            break;
        case ICLDB__ALREADY_KEY:
            icldb__errorMsg = "Key already exists";
            break;
        case ICLDB__SLOTS_ALLOC:
            icldb__errorMsg = "Couldn't allocate more slots. Out of mem?";
            break;
        default:
            icldb__errorMsg = "Ok";
    }
    return icldb__errorMsg;
}








/*
 ------------------------------------------------------------------------------
    Tests
 ------------------------------------------------------------------------------
*/

#if !defined(DISABLE_TESTS)

constexpr unsigned icldb__testBitvec()
{
    unsigned bitvecBits[3] = {0};
    icldb__bitvec testBitvec;
    testBitvec.bitvec = bitvecBits;
    for (int i=0; i<64; ++i)
        icldb__bitvecSet(&testBitvec, i);
    const bool TEST_BIT_63 =   icldb__bitvecCheck(&testBitvec, 63)
                           && !icldb__bitvecCheck(&testBitvec, 64);
    icldb__bitvecSet(&testBitvec, 64);
    const bool TEST_BIT_64 =   icldb__bitvecCheck(&testBitvec, 64)
                           && !icldb__bitvecCheck(&testBitvec, 65);
    
    return  TEST_BIT_63
         | (TEST_BIT_64 << 1);
}
constexpr unsigned resBitvec = icldb__testBitvec();

static_assert(resBitvec&1, "bit 63 not correct");
static_assert(resBitvec&2, "bit 64 not correct");


constexpr unsigned icldb__testSlots()
{
    includeDB testDB{ .mappedArray=nullptr,
                      .chunkSize=0,
                      .nodeVec=nullptr,
                      .seed=0
                    };
    unsigned bitvecBits[3/* *32 */] = {0};
    testDB.occupied.bitvec = bitvecBits;
    testDB.occupied.szVecIn32Chunks  = 1;
    auto testAlloc = [](icldb__bitvec *bitvec, int amount)->int
                     {
                         if ((bitvec->szVecIn32Chunks+amount) <= 3)
                         {
                             bitvec->szVecIn32Chunks += amount;
                             return 0;
                         }
                         return 1;
                     };
    testDB.occupied.icldb__bitvecAlloc = testAlloc;
    const bool TEST_HAVE_ZERO_SLOTS = icldb__gatherSlots(&testDB, 0) == 0;
    int location = icldb__gatherSlots(&testDB, 2);
    icldb__markSlots(&testDB.occupied, location, 2);
    const bool TEST_TWO_SLOTS_OCCUPIED =  (bitvecBits[0]&1)
                                       && (bitvecBits[0]&2)
                                       && !(bitvecBits[0]&4)
                                       && location==0;
    location = icldb__gatherSlots(&testDB, 1);
    icldb__markSlots(&testDB.occupied, location, 1);
    const bool TEST_LOCATION_TWO = location == 2;
    location = icldb__gatherSlots(&testDB, 7);
    icldb__markSlots(&testDB.occupied, location, 7);
    const bool TEST_LOCATION_THREE = location == 3;
    // test large allocation:
    location = icldb__gatherSlots(&testDB, 24+39);
    icldb__markSlots(&testDB.occupied, location, 24+39);
    const bool TEST_LOCATION_EIGHT = location == 10;
    location = icldb__gatherSlots(&testDB, 22);
    icldb__markSlots(&testDB.occupied, location, 22);
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
constexpr unsigned resSlots = icldb__testSlots();

static_assert(resSlots&  1, "couldn't gather 0 slots");
static_assert(resSlots&  2, "slots were not correctly marked as 'occupied'");
static_assert(resSlots&  4, "incorrect slot position (2)");
static_assert(resSlots&  8, "incorrect slot position (3)");
static_assert(resSlots& 16, "incorrect slot position (8)");
static_assert(resSlots& 32, "incorrect slot position (8+24+39)");
static_assert(resSlots& 64, "flags set incorrectly");


constexpr unsigned icldb__testSkiplist()
{
    // setup:
    icldb__skipnode testNodes[32];
    includeDB testDB{ .mappedArray=nullptr,
                      .chunkSize=0,
                      .nodeVec=testNodes,
                      .icldb__nodevecAlloc=[](includeDB *instance, int newSize)->int
                                           {
                                               (void)instance;
                                               (void)newSize;
                                               return 0;
                                           },
                      .seed=0
                    };
    // start testing:
    icldb__insertNewSkipnode(&testDB, 111, 0);
    const bool TEST_HEAD_IS_ZERO = testDB.headD == 0;
    icldb__insertNewSkipnode(&testDB, 333, 1);
    const bool TEST_NODE_ADD =  testNodes[0].nodeid == 111
                             && testNodes[testDB.headD].nodeid == 111
                             && testDB.headD == 0
                             && testNodes[0].next == 1
                             && testNodes[1].next == 0
                             && testNodes[testNodes[0].next].nodeid == 333;
    const bool TEST_NODES_POINTING_TO_CORRECT_POSITION =  testNodes[0].filepos == 0
                                                       && testNodes[1].filepos == 1;
    // insert node between:
    icldb__insertNewSkipnode(&testDB, 222, 2);
    const bool TEST_NODE_BETWEEN =  testNodes[2].nodeid == 222
                                 && testNodes[2].next == 1
                                 && testNodes[1].next == 0
                                 && testNodes[0].next == 2;
    // find nodes:
    const icldb__skipnode *first  = icldb__findSkipnode(&testDB, 111);
    const icldb__skipnode *second = icldb__findSkipnode(&testDB, 333);
    const icldb__skipnode *third  = icldb__findSkipnode(&testDB, 222);
    const bool TEST_NODES_FOUND =  first && second && third
                                && first->filepos==0 && second->filepos==1 && third->filepos==2;
    // add node to end:
    icldb__insertNewSkipnode(&testDB, 444, 3);
    const icldb__skipnode *found = icldb__findSkipnode(&testDB, 444);
    const bool TEST_NODE_AT_END =  found && found->filepos == 3
                                && testNodes[3].nodeid == 444
                                && testNodes[1].next == 3
                                && testNodes[3].next == 0;
    // not found:
    const bool TEST_NODE_NOT_FOUND = icldb__findSkipnode(&testDB, 69) == nullptr;
    
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
constexpr unsigned resList = icldb__testSkiplist();

static_assert(resList&  1, "head pointing to wrong place");
static_assert(resList&  2, "node was not correctly inserted");
static_assert(resList&  4, "nodes did not sit where they were supposed to");
static_assert(resList&  8, "nodes not correctly sorted");
static_assert(resList& 16, "nodes could not be recovered");
static_assert(resList& 32, "node at end not found");
static_assert(resList& 64, "icldb__findSkipnode() did not return NULL");


constexpr unsigned icldb__testDBOps()
{
    // setup:
    unsigned char testBuffer[33*64] = {0};
    unsigned char fakeFile[4*32*64] = {0};
    icldb__skipnode testNodes[3*32];
    includeDB testDB{ .buffer=testBuffer,
                      .szBuf=sizeof(testBuffer),
                      .mappedArray=fakeFile,
                      .chunkSize=64,
                      .nodeVec=testNodes,
                      .icldb__nodevecAlloc=[](includeDB *instance, int newSize)->int
                                           {
                                               (void)instance;
                                               (void)newSize;
                                               return 0;
                                           },
                      .seed=6969
                     };
    auto testWrite = [](includeDB *instance, int location, int amount)->int
                     {
                         unsigned char *data = &instance->mappedArray[location];
                         for (int i=0; i<amount; ++i)
                             data[i] = instance->buffer[i];
                         return 0; // success
                     };
    auto testRead = [](includeDB *instance, int location, int amount)
                    {
                        unsigned char *data = &instance->mappedArray[location];
                        for (int i=0; i<amount; ++i)
                            instance->buffer[i] = data[i];
                        return 0; // success
                    };
    testDB.icldb__write = testWrite;
    testDB.icldb__read = testRead;
    unsigned bitvecBits[4/* *32 */] = {0};
    testDB.occupied.bitvec = bitvecBits;
    testDB.occupied.szVecIn32Chunks = 1;
    testDB.occupied.icldb__bitvecAlloc = [](icldb__bitvec *bitvec, int amount)->int
                                         {
                                             if ((bitvec->szVecIn32Chunks + amount) <= 4)
                                             {
                                                 bitvec->szVecIn32Chunks += amount;
                                                 return 0;
                                             }
                                             return 1;
                                         };
    // put:
    unsigned char keyA[6] = "hello";
    unsigned char valA[55] = "wurld 1111111111222222222233333333334444444444abcdef.,";
    const bool TEST_PUT_SUCCESS_A = icldb_put(&testDB, keyA, 6, valA, 55) == 0;
    const bool TEST_SHOULD_BE_W =  fakeFile[16 + sizeof(keyA)] == 'w'
                                && fakeFile[20 + sizeof(keyA)] == 'd'
                                && fakeFile[22 + sizeof(keyA)] == '1'
                                && fakeFile[68 + sizeof(keyA)] == '.';
    const bool TEST_TWO_SLOTS_TAKEN =  icldb__bitvecCheck(&testDB.occupied, 0)
                                    && icldb__bitvecCheck(&testDB.occupied, 1);
    const bool TEST_NODE_CREATED_CORRECTLY =  testDB.nodeVec[0].nodeid == icldb__xx32(keyA, 6, testDB.seed)
                                           && testDB.nodeVec[0].next == 0;
    // get:
    const unsigned char *resA = icldb_get(&testDB, keyA, 6, nullptr);
    const bool TEST_GET_SUCCESS_A = resA && (resA[0]=='w') && (resA[50]=='e') && (resA[51]=='f') && (resA[52]=='.') && (resA[53]==',');
    // put again:
    unsigned char keyB[5] = "lexi";
    unsigned char valB[5] = "paxi";
    const bool TEST_PUT_SUCCESS_B = icldb_put(&testDB, keyB, 5, valB, 5) == 0;
    const bool TEST_SHOULD_BE_P =  fakeFile[64+64+16 + sizeof(keyB)] == 'p'
                                && fakeFile[64+64+16+3 + sizeof(keyB)] == 'i';
    const bool TEST_THREE_SLOTS_TAKEN =  icldb__bitvecCheck(&testDB.occupied, 0)
                                      && icldb__bitvecCheck(&testDB.occupied, 1)
                                      && icldb__bitvecCheck(&testDB.occupied, 2)
                                      && !icldb__bitvecCheck(&testDB.occupied, 3)
                                      && !icldb__bitvecCheck(&testDB.occupied, 32);
    const bool TEST_SECOND_NODE =  testDB.nodeVec[0].nodeid == icldb__xx32(keyA, 6, testDB.seed)
                                && testDB.nodeVec[1].nodeid == icldb__xx32(keyB, 5, testDB.seed);
    // get again:
    int vallen=0;
    const unsigned char *resB = icldb_get(&testDB, keyB, 5, &vallen);
    const bool TEST_GET_SUCCESS_B = resB && resB[0]=='p' && vallen==5;
    // get s/thing that doesn't exist:
    unsigned char keyC[5] = "😕";
    const unsigned char *notFound = icldb_get(&testDB, keyC, 5, nullptr);
    const bool TEST_GET_FAILED = notFound == nullptr;
    // put many:
    bool didPut = true;
    unsigned char keyD[6] = "fixi_";
    for (int i=0; i<30; ++i)
    {
        keyD[4] = i+'0';
        const unsigned char *valD = &keyD[4];
        didPut = (icldb_put(&testDB, keyD, 6, valD, 1) == 0) && didPut;
    }
    const bool TEST_PUT_MANY_SUCCESS =  didPut
                                     && icldb__bitvecCheck(&testDB.occupied, 3)
                                     && icldb__bitvecCheck(&testDB.occupied, 32)
                                     && !icldb__bitvecCheck(&testDB.occupied, 33);
    // get last:
    const unsigned char *oldkey = icldb_get(&testDB, keyA, 6, nullptr);
    const unsigned char *last = icldb_get(&testDB, keyD, 6, nullptr);
    const bool TEST_GET_MANY_SUCCESS = last && last[0] == keyD[4];
    // get all:
    bool didGet = true;
    for (int i=0; i<30; ++i)
    {
        keyD[4] = i+'0';
        const unsigned char *valD = icldb_get(&testDB, keyD, 6, nullptr);
        didGet = didGet && valD && (valD[0] == i+'0');
    }
    const bool TEST_GET_MANY_SUCCESS_PART2 = didGet;
    // put very big value:
    unsigned char keyE[18] = "hello! I'm BIIIIG";
    unsigned char valE[32*64+1] = {0};
    valE[32*64] = 'x';
    const bool TEST_PUT_BIG_VALUE = icldb_put(&testDB, keyE, 18, valE, 32*64+1) == 0;
    // make sure all bits marked correctly:
    bool marked = true;
    for (int i=0; i<34; ++i)
        marked = icldb__bitvecCheck(&testDB.occupied, 1+1+30+i) && marked;
    const bool TEST_33_SLOTS_TAKEN = marked;
    // get 'big':
    const unsigned char *resE = icldb_get(&testDB, keyE, 18, &vallen);
    const bool TEST_BIG_VAL = resE && (vallen == 32*64+1) && resE[32*64]=='x' && resE[32*64-1]!='x';
    // check if cursor points to first record:
    int keylen = 0;
    unsigned char *curKey = icldb_curGet(&testDB, &keylen);
    const bool TEST_KEY_AT_CURSOR =  curKey
                                  && (keylen == 6)
                                  && curKey[0] == 'h'
                                  && curKey[4] == 'o';
    // shift cursor
    icldb_next(&testDB);
    curKey = icldb_curGet(&testDB, &keylen);
    const bool TEST_KEY_AFTER_CALLING_NEXT =  curKey
                                           && (keylen == 5)
                                           && curKey[0] == 'l'
                                           && curKey[3] == 'i';
    for (int i=0; i<31; ++i)
        icldb_next(&testDB);
    curKey = icldb_curGet(&testDB, &keylen);
    const bool TEST_LAST_KEY_AFTER_CALLING_NEXT =  curKey
                                                && (keylen == 18)
                                                && curKey[0] == 'h'
                                                && curKey[16] == 'G';
    // cursor end
    icldb_next(&testDB);
    curKey = icldb_curGet(&testDB, &keylen);
    const bool TEST_CURSOR_END = (keylen == 0) && (curKey == nullptr);
    // double put
    const bool TEST_DOUBLE_PUT_SHOULD_FAIL =  icldb_put(&testDB, keyB, 5, valB, 5) == 1
                                           && testDB.ec == ICLDB__ALREADY_KEY;
    
    
    return  TEST_PUT_SUCCESS_A
         | (TEST_SHOULD_BE_W << 1)
         | (TEST_TWO_SLOTS_TAKEN << 2)
         | (TEST_NODE_CREATED_CORRECTLY << 3)
         | (TEST_GET_SUCCESS_A << 4)
         | (TEST_PUT_SUCCESS_B << 5)
         | (TEST_SHOULD_BE_P << 6)
         | (TEST_THREE_SLOTS_TAKEN << 7)
         | (TEST_SECOND_NODE << 8)
         | (TEST_GET_SUCCESS_B << 9)
         | (TEST_GET_FAILED << 10)
         | (TEST_PUT_MANY_SUCCESS << 11)
         | (TEST_GET_MANY_SUCCESS << 12)
         | (TEST_GET_MANY_SUCCESS_PART2 << 13)
         | (TEST_PUT_BIG_VALUE << 14)
         | (TEST_33_SLOTS_TAKEN << 15)
         | (TEST_BIG_VAL << 16)
         | (TEST_KEY_AT_CURSOR << 17)
         | (TEST_KEY_AFTER_CALLING_NEXT << 18)
         | (TEST_LAST_KEY_AFTER_CALLING_NEXT << 19)
         | (TEST_DOUBLE_PUT_SHOULD_FAIL << 20)
         ;
}
constexpr unsigned resTestOps = icldb__testDBOps();

static_assert(resTestOps&     1 , "icldb_put() failed");
static_assert(resTestOps&(1<< 1), "the fakeFile did not contain correct data after icldb_put()");
static_assert(resTestOps&(1<< 2), "last two bits were not marked 'occupied' after icldb_put()");
static_assert(resTestOps&(1<< 3), "node incorrect");
static_assert(resTestOps&(1<< 4), "icldb_get() did not return 'wurld'");
static_assert(resTestOps&(1<< 5), "icldb_put() failed with 'lexi'");
static_assert(resTestOps&(1<< 6), "the fakeFile did not contain correct data after another icldb_put()");
static_assert(resTestOps&(1<< 7), "three slots should now be occupied");
static_assert(resTestOps&(1<< 8), "second node not correct");
static_assert(resTestOps&(1<< 9), "second key/val incorrect");
static_assert(resTestOps&(1<<10), "should be NULL, instead some value was returned!");
static_assert(resTestOps&(1<<11), "could not icldb_put() all the keys");
static_assert(resTestOps&(1<<12), "icldb_get() failed to return desired value");
static_assert(resTestOps&(1<<13), "at least one value returned did not match the original data");
static_assert(resTestOps&(1<<14), "could not icldb_put() big value");
static_assert(resTestOps&(1<<15), "not all slots marked 'occupied'");
static_assert(resTestOps&(1<<16), "icldb_get() did not return a large value correctly");
static_assert(resTestOps&(1<<17), "the key at the cursor position should be the first key");
static_assert(resTestOps&(1<<18), "incorrect key returned after calling icldb_next()");
static_assert(resTestOps&(1<<19), "incorrect key at last cursor position");
static_assert(resTestOps&(1<<20), "icldb_put()ing the same key twice should produce a ICLDB__ALREADY_KEY error code");


constexpr unsigned icldb__testDBput()
{
    // 25*256 == 6400
    unsigned char testBuffer[25*256] = {0};
    unsigned char fakeFile[(25*256)+(25*256)+(25*256)] = {0};
    icldb__skipnode testNodes[3];
    includeDB anotherTestDB{ .buffer=testBuffer,
                             .szBuf=sizeof(testBuffer),
                             .mappedArray=fakeFile,
                             .chunkSize=256,
                             .nodeVec=testNodes,
                             .icldb__nodevecAlloc=[](includeDB *instance, int newSize)->int
                                                  {
                                                      (void)instance;
                                                      (void)newSize;
                                                      return 0;
                                                  },
                             .seed=4242
                           };
    auto testWrite = [](includeDB *instance, int location, int amount)->int
                     {
                         unsigned char *data = &instance->mappedArray[location];
                         for (int i=0; i<amount; ++i)
                             data[i] = instance->buffer[i];
                         return 0; // success
                     };
    auto testRead = [](includeDB *instance, int location, int amount)
                    {
                        unsigned char *data = &instance->mappedArray[location];
                        for (int i=0; i<amount; ++i)
                            instance->buffer[i] = data[i];
                        return 0; // success
                    };
    anotherTestDB.icldb__write = testWrite;
    anotherTestDB.icldb__read = testRead;
    unsigned bitvecBits[3/* *32 */] = {0};
    anotherTestDB.occupied.bitvec = bitvecBits;
    anotherTestDB.occupied.szVecIn32Chunks = 1;
    auto testAlloc = [](icldb__bitvec *bitvec, int amount)->int
                     {
                         if ((bitvec->szVecIn32Chunks + amount) <= 3)
                         {
                             bitvec->szVecIn32Chunks += amount;
                             return 0;
                         }
                         return 1;
                     };
    anotherTestDB.occupied.icldb__bitvecAlloc = testAlloc;
    
    auto makeKey = [](unsigned char *key, const unsigned src)
    {
        key[0] = (src>>24)&0xff;
        key[1] = (src>>16)&0xff;
        key[2] = (src>> 8)&0xff;
        key[3] =  src     &0xff;
        key[4] = '.';
        key[5] = '.';
    };
    
    unsigned char key[6] = {0};
    makeKey(key, 12345678);
    unsigned char val[6144] = {0};
    for (int i=0; i<6144; ++i)
        val[i] = '.';
    
    const bool TEST_PUT_SUCCESS_01 = icldb_put(&anotherTestDB, key, 6, val, 6144) == 0;
    unsigned char *res01 = icldb_get(&anotherTestDB, key, 6, nullptr);
    const bool TEST_GET_SUCCESS_01 = res01 != nullptr;
    
    makeKey(key, 87654321);
    
    const bool TEST_PUT_SUCCESS_02 = icldb_put(&anotherTestDB, key, 6, val, 6144) == 0;
    unsigned char *res02 = icldb_get(&anotherTestDB, key, 6, nullptr);
    const bool TEST_GET_SUCCESS_02 = res02 != nullptr;
    
    makeKey(key, 12343210);
    val[0] = 'a'; val[6143] = 'z';
 
    const bool TEST_PUT_SUCCESS_03 = icldb_put(&anotherTestDB, key, 6, val, 6144) == 0;
    unsigned char *res03 = icldb_get(&anotherTestDB, key, 6, nullptr);
    const bool TEST_GET_SUCCESS_03 = res03 != nullptr;
    
    const bool TEST_VALUES_ARE_CORRECT =  res03
                                       && res03[0] == 'a'
                                       && res03[6143] == 'z';
    
    return  TEST_PUT_SUCCESS_01
         | (TEST_GET_SUCCESS_01 << 1)
         | (TEST_PUT_SUCCESS_02 << 2)
         | (TEST_GET_SUCCESS_02 << 3)
         | (TEST_PUT_SUCCESS_03 << 4)
         | (TEST_PUT_SUCCESS_03 << 5)
         | (TEST_VALUES_ARE_CORRECT << 6)
         ;
}
constexpr unsigned resTestPut = icldb__testDBput();

static_assert(resTestPut&  1, "did not put key (1)");
static_assert(resTestPut&  2, "did not get key (1)");
static_assert(resTestPut&  4, "did not put key (2)");
static_assert(resTestPut&  8, "did not get key (2)");
static_assert(resTestPut& 16, "did not put key (3)");
static_assert(resTestPut& 32, "did not get key (3)");
static_assert(resTestPut& 64, "res03 values incorrect");

#endif // !defined(DISABLE_TESTS)

#endif // INCLUDEDB_IMPLEMENTATION


#endif // INCLUDEDB_DB_H

/*
 ------------------------------------------------------------------------------
 Copyright (c) 2021 Professor Peanut
 
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
