#define INCLUDEDB_IMPLEMENTATION
#define INCLUDEDB_CHUNK_SIZE 128 /* optional */
#include "includedb.h"

#include <stdio.h>

int main(void)
{
    // Create new file or open existing:
    includeDB *db = includedb_open("hello.dat");
    if (!db)
    {
        printf("error opening db\n");
    }
    
 /*   
    // Iterate over all keys:
    int keylen, vallen;
    unsigned char *curKey = icldb_curGetKey(db, &keylen);
    char *key, *record;
    while (curKey)
    {
        // Copy key to show later:
        key = (char *)realloc(key, keylen+1);
        memcpy(key, curKey, keylen);
        key[keylen] = '\0';
        
        const unsigned char *val = icldb_get(db, curKey, keylen, &vallen);
        
        // Copy value to display:
        record = (char *)realloc(record, vallen+1);
        memcpy(record, val, vallen);
        record[vallen] = '\0';
    
        // Show:
        printf("key: %s, val: %s\n", key, val);
        
        // Advance cursor:
        icldb_next(db);
        
        // Get another key:
        curKey = icldb_curGetKey(db, &keylen);
    }
    free(record);
    free(key);
    
    // Put new record:
    const unsigned char newkey[] = {'h','e','l','l','o'};
    const unsigned char newval[] = {'w','o','r','l','d'};
    if (icldb_put(db, newkey, 5, newval, 5) == 1)
    {
        printf("Error: %s \n", icldb_getError(db));
    }
    
    // Done:
    includedb_close(db);
    */
}


