#include <string.h> /* memcpy() */
#include <stdio.h>

#define INCLUDEDB_IMPLEMENTATION
#define INCLUDEDB_CHUNK_SIZE 32 /* optional */
#include "includedb.h"


int main(void)
{
    // Create new file or open existing:
    includeDB *db = includedb_open("hello.dat");
    if (!db)
    {
        printf("error opening db\n");
    }
    
   
    // Iterate over all keys:
    int keylen, vallen;
    unsigned char *curKey = includedb_curGetKey(db, &keylen);
    char *key=nullptr, *record=nullptr;
    while (curKey)
    {
        // Copy key to show later:
        key = (char *)realloc(key, keylen+1);
        memcpy(key, curKey, keylen);
        key[keylen] = '\0';
        
        const unsigned char *val = includedb_get(db, curKey, keylen, &vallen);
        
        // Copy value to display:
        record = (char *)realloc(record, vallen+1);
        memcpy(record, val, vallen);
        record[vallen] = '\0';
    
        // Show:
        printf("key: %s, val: %s\n", key, val);
        
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
    {
        printf("Can't put %s: %s \n", newkey_1, includedb_getError(db));
    }
    const unsigned char newkey_2[] = {'h','e','l','l','o','2','\0'};
    const unsigned char newval_2[] = {'w','o','r','l','d','2','\0'};
    if (includedb_put(db, newkey_2, sizeof(newkey_2), newval_2, sizeof(newval_2)) == 1)
    {
        printf("Can't put %s: %s \n", newkey_2, includedb_getError(db));
    }
    
    // Done:
    includedb_close(db);
}


