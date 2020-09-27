#define INCLUDEDB_IMPLEMENTATION
#include "includedb.h"

int main(void)
{
    // Create new file or open existing:
    nanopulseDB *db = nplse_open("hello.dat");
    if (!db)
    {
        printf("error opening db\n");
    }
    
    
    // Iterate over all keys:
    int keylen;
    unsigned char *curKey = nplse_curGet(db, &keylen);
    while (curKey)
    {
    
    
        // Advance cursor:
        nplse_next(db);
    }
    
    
    // Done:
    nplse_close(db);
}
