#include "includedb_bitvec.h"

static int includedb__bitvecAlloc(includedb__bitvec *bitvec, int amount)
{
    const int szOld = bitvec->szVecIn32Chunks;
    bitvec->szVecIn32Chunks += amount;
    unsigned *tmp = (unsigned *)includedb__zeroalloc(bitvec->szVecIn32Chunks, sizeof(unsigned));
    if (!tmp)
        return includedb__error;
    for (int i=0; i<szOld; ++i)
        tmp[i] = bitvec->bitvec[i];
    includedb__free(bitvec->bitvec);
    bitvec->bitvec = tmp;
    return includedb__ok;
}

inline constexpr unsigned includedb__bitvecCheck(const includedb__bitvec *bitvec, int pos)
{
    const unsigned bit = bitvec->bitvec[pos>>5];
    return (bit >> (pos&31)) & 1;
}

inline constexpr void includedb__bitvecSet(includedb__bitvec *bitvec, int pos)
{
    const unsigned bit = 1 << (pos&31);
    bitvec->bitvec[pos>>5] |= bit;
}


/*++++++++++++++++++++++++++++++++++++++*/
/*                                Tests */
/*++++++++++++++++++++++++++++++++++++++*/
#if !defined(DISABLE_TESTS)

constexpr unsigned includedb__testBitvec()
{
    unsigned bitvecBits[3] = {0};
    includedb__bitvec testBitvec;
    testBitvec.bitvec = bitvecBits;
    for (int i=0; i<64; ++i)
        includedb__bitvecSet(&testBitvec, i);
    const bool TEST_BIT_63 =   includedb__bitvecCheck(&testBitvec, 63)
                           && !includedb__bitvecCheck(&testBitvec, 64);
    includedb__bitvecSet(&testBitvec, 64);
    const bool TEST_BIT_64 =   includedb__bitvecCheck(&testBitvec, 64)
                           && !includedb__bitvecCheck(&testBitvec, 65);
    
    return  TEST_BIT_63
         | (TEST_BIT_64 << 1);
}
constexpr unsigned resBitvec = includedb__testBitvec();

static_assert(resBitvec&1, "bit 63 not correct");
static_assert(resBitvec&2, "bit 64 not correct");

#endif // !defined(DISABLE_TESTS)

