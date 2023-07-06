#include "includedb_skiplist.h"

static int includedb__nodevecAlloc(includeDB *instance, int newSize)
{
    includedb__skipnode *tmp = (includedb__skipnode *)includedb__realloc(instance->nodeVec, newSize * sizeof(includedb__skipnode));
    if (!tmp)
        return includedb__error;
    instance->nodeVec = tmp;
    return includedb__ok;
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


