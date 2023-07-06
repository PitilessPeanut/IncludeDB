#include "includedb_core.h"


static includeDB *includedb_open(const char *filename)
{
    COMPTIME unsigned char magic[] = "incldedb";
    COMPTIME unsigned v = INCLUDEDB_VER_NUM;
    COMPTIME unsigned char versionStr[] = {(v>>24)&0xff, (v>>16)&0xff, (v>>8)&0xff, v&0xff};
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
        includedb__fileWrite(&newInstance->file, versionStr, 4, 8);
        // write chunksize
        COMPTIME unsigned cs = INCLUDEDB_CHUNK_SIZE;
        COMPTIME unsigned char chunksize[] = {(cs>>24)&0xff, (cs>>16)&0xff, (cs>>8)&0xff, cs&0xff};
        includedb__fileWrite(&newInstance->file, chunksize, 4, 12);
        newInstance->chunkSize = cs;
        // write keys & "visits"
        COMPTIME unsigned char keysNvisits[] = {0,0,0,0,0,0,0,0};
        includedb__fileWrite(&newInstance->file, keysNvisits, 8, 16);
        newInstance->globalVisits = 0;
        // create seed
        const unsigned sd = 6969; // todo get from time
        const unsigned char seedStr[] = {(sd>>24)&0xff, (sd>>16)&0xff, (sd>>8)&0xff, sd&0xff};
        includedb__fileWrite(&newInstance->file, seedStr, 4, 24);
        newInstance->seed = sd;
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
        ok =       buf[ 8] == INCLUDEDB_VER_MAJOR;
        ok = ok && buf[ 9] <= INCLUDEDB_VER_MINOR;
        ok = ok && buf[10] <= INCLUDEDB_VER_PATCH_A;
        ok = ok && buf[11] <= INCLUDEDB_VER_PATCH_B;
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
    /*
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
        
        
        const unsigned priA=buf[12], priB=buf[13], priC=buf[14], priD=buf[15];
        const unsigned tombstone = (priA<<24) | (priB<<16) | (priC<< 8) | priD;
        printf("%d - hash: %d tomb: %d \n", i, keyhash, tombstone);
        
        
        
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
    */
}

