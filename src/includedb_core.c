#include "includedb_core.h"


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
        written = includedb__fileWrite(&newInstance->file, keysNvisits, 8, 16);
        newInstance->globalVisits = 0;
        // create seed
        const unsigned sd = 6987; // todo get from time
        const unsigned char seedStr[] = {(sd>>24)&0xff, (sd>>16)&0xff, (sd>>8)&0xff, sd&0xff};
        written = includedb__fileWrite(&newInstance->file, seedStr, 4, 24);
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
        printf("vers %d %d %d %d \n", buf[8],buf[9],buf[10],buf[11]);
        // read chunksise
        newInstance->chunkSize = (buf[12]<<24) | (buf[13]<<16) | (buf[14]<<8) | buf[15];
        printf("cksz %d  %d %d %d %d \n", newInstance->chunkSize, buf[12],buf[13],buf[14],buf[15]);
        // read # of keys
        nKeys = (buf[16]<<24) | (buf[17]<<16) | (buf[18]<<8) | buf[19];
        printf("nKeys %d  \n", nKeys);
        // read global visits
          // todo: not yet
        // read randseed
        newInstance->seed = (buf[24]<<24) | (buf[25]<<16) | (buf[26]<<8) | buf[27];
        printf("seed %d  %d %d %d %d \n", newInstance->seed, buf[24],buf[25],buf[26],buf[27]);
        
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
    
    // Init heads
    newInstance->head[0] = 0;
    newInstance->head[1] = 0;
    newInstance->head[2] = 0;
    newInstance->head[3] = 0;
    
    newInstance->nKeys = 0;
    // put cursor to the start
    newInstance->cursor = 0;
    // init bloomfilter (todo)
    //newInstance->bloommap = 0;
    //for (int i=0; i<szBloommap; ++i)
    //    newInstance->bloomcounters[i] = 0;
    // reset error
    newInstance->ec = INCLUDEDB__OK;
    
    
    printf("num ky %d \n", nKeys);
    
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
        printf("%d - hash: %d tomb: %s \n", i, keyhash, tombstone==0?"yes":"no");
        
        
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
    includedb__fileWrite(&instance->file, keysNvisits, 8, 16);
    printf("clse k : %d \n", nk);
    
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

