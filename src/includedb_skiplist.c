#include "includedb_skiplist.h"

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


