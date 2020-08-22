#define NANOPULSE_DB_IMPLEMENTATION
#include "nanopulsedb.h"

int main(void)
{
    // Create new file or open existing:
    nanopulseDB *db = nplse_open("hello.dat");
    if (!db)
        printf("error opening db\n");
    
    nplse_close(db);
}
