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

    // Error code
    enum includedb__errorCodes ec;
} includeDB;

static constexpr int includedb__ok = 0;
static constexpr int includedb__error = 1;




/*
 ------------------------------------------------------------------------------
    Public interface
    All operations are synchronous!! 
 ------------------------------------------------------------------------------
*/
// Open existing, or create new
static includeDB *includedb_open(const char *filename);




#endif // INCLUDEDB_CONFIG_H
