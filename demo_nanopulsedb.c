#define NANOPULSE_DB_IMPLEMENTATION
#include "nanopulsedb.h"

int main(void)
{
    nanopulseDB *db = nplse_open("hello.txt");
    if (!db)
        printf("error opening db\n");
    
    nplse_close(db);
}
