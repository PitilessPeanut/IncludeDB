#define NANOPULSE_DB_IMPLEMENTATION
#include "nanopulsedb.h"

int main(void)
{
    nanopulseDB *db = nplse_open("hello.dat");
    
    nplse_close(db);
}
