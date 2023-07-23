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


#ifndef INCLUDEDB_LOCKS
  #if defined(__cplusplus)
    #include <shared_mutex>
  #else
    #if defined(_WIN32)
    #else
      #include <pthread.h>
    #endif
  #endif
  #define INCLUDEDB_LOCKS
#endif


#ifndef INCLUDEDB_NLAYERS
  // Increase this if you access the same keys more often. Decrease if you
  // like to access a larger number of different keys
  #define INCLUDEDB_NLAYERS 4
#endif


#if !defined(INCLUDEDB_CHUNK_SIZE)
  #define INCLUDEDB_CHUNK_SIZE 256
#endif


struct includedb__file;

bool includedb__fileOpen(struct includedb__file *file, const char *filename);
bool includedb__fileCreate(struct includedb__file *file, const char *filename);
bool includedb__fileClose(struct includedb__file *file);
int  includedb__fileWrite(struct includedb__file *file, const unsigned char *bytes, int len, int filepos);
void includedb__fileRead(struct includedb__file *file, unsigned char *bytes, int len, int filepos);
bool includedb__fileGrow(struct includedb__file *file, int len);

#define INCLUDEDB_USE_STD_FILE_FALLBACK // todo: temporary. This should be removed after better file-acces is implemented
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

// Error:
static const char *includedb__errorMsg;


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
    int head[INCLUDEDB_NLAYERS];
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
    (All operations are synchronous!!)
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
