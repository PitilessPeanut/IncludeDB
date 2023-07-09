#include <string.h> /* memcpy() */
#include <stdio.h>
#include <assert.h>

#define INCLUDEDB_IMPLEMENTATION
#define INCLUDEDB_CHUNK_SIZE 32 /* optional */
#include "includedb.h"


int main(void)
{
    // Create new file or open existing:
    includeDB *db = includedb_open("hello.dat");
    if (!db)
    {
        printf("error opening db: %s\n", includedb_getError(db));
        return -1;
    }

    printf("Open db: success\n");
    
    int keylen, vallen, nSuccess=0;
    unsigned char *curKey = includedb_curGetKey(db, &keylen);
    
    char *key=NULL, *record=NULL;
    // Iterate over all keys:
    while (curKey)
    {
        // Copy key to show later:
        key = (char *)realloc(key, keylen+1);
        memcpy(key, curKey, keylen);
        key[keylen] = '\0';
        
        const unsigned char *val = includedb_get(db, curKey, keylen, &vallen);
        nSuccess += val != NULL;
        
        // Copy value to display:
        record = (char *)realloc(record, vallen+1);
        memcpy(record, val, vallen);
        record[vallen] = '\0';
    
        // Show:
        printf("key found: %s, val: %s\n", key, val);
        
        // Advance cursor:
        includedb_next(db);
        
        // Get another key:
        curKey = includedb_curGetKey(db, &keylen);
    }
    free(record);
    free(key);
    
    // Put new records:
    const unsigned char newkey_1[] = {'h','e','l','l','o','1','\0'};
    const unsigned char newval_1[] = {'w','o','r','l','d','1','\0'};
    if (includedb_put(db, newkey_1, sizeof(newkey_1), newval_1, sizeof(newval_1)) == 1)
        printf("Can't put %s: %s \n", newkey_1, includedb_getError(db));
    const unsigned char newkey_2[] = {'h','e','l','l','o','2','\0'};
    const unsigned char newval_2[] = {'w','o','r','l','d','2','\0'};
    if (includedb_put(db, newkey_2, sizeof(newkey_2), newval_2, sizeof(newval_2)) == 1)
        printf("Can't put %s: %s \n", newkey_2, includedb_getError(db));
    
    if (nSuccess==0)
    {
        includedb_close(db); // Don't forget!!
        return 0;
    }
        
    // Read records:
    int world1_len;
    const unsigned char *world1 = includedb_get(db, newkey_1, sizeof(newkey_1), &world1_len);
    assert(world1_len == sizeof("world1"));
    int ok = 1;
    ok = ok && (world1[0] == 'w');
    ok = ok && (world1[1] == 'o');
    ok = ok && (world1[2] == 'r');
    ok = ok && (world1[3] == 'l');
    ok = ok && (world1[4] == 'd');
    ok = ok && (world1[5] == '1');
    assert(ok > 0);

    printf("Global visits: %d, hello1 visits: %d, hello2 visits: %d \n", db->globalVisits, db->nodeVec[0].visits, db->nodeVec[1].visits);
    
    // Done:
    includedb_close(db);
}


 
