#include "includedb_bloom.h"

typedef unsigned includedb__inttype;

#define N_SLOTS (sizeof(includedb__inttype) * 8)


typedef struct includedb__bloom
{
    CTOR2(includedb__bloom() : counters{0}, bitmap(0) {})
    int counters[N_SLOTS];

    includedb__inttype bitmap;
} includedb__bloom;


constexpr void includedb__bloomPut(includedb__bloom *bloom, includedb__inttype hash)
{
    bloom->bitmap |= hash;
    for (size_t i=0; i<N_SLOTS; i+=8)
    {
        bloom->counters[i+0] += hash & 1;  hash >>= 1;
        bloom->counters[i+1] += hash & 1;  hash >>= 1;
        bloom->counters[i+2] += hash & 1;  hash >>= 1;
        bloom->counters[i+3] += hash & 1;  hash >>= 1;
        bloom->counters[i+4] += hash & 1;  hash >>= 1;
        bloom->counters[i+5] += hash & 1;  hash >>= 1;
        bloom->counters[i+6] += hash & 1;  hash >>= 1;
        bloom->counters[i+7] += hash & 1;  hash >>= 1;
    }
}

constexpr void includedb__bloomRemove(includedb__bloom *bloom, includedb__inttype hash)
{
    includedb__inttype mask = 0;
    for (size_t i=0; i<N_SLOTS; i+=8)
    {
        bloom->counters[i+0] -= hash & 1; hash >>= 1;
        bloom->counters[i+1] -= hash & 1; hash >>= 1;
        bloom->counters[i+2] -= hash & 1; hash >>= 1;
        bloom->counters[i+3] -= hash & 1; hash >>= 1;
        bloom->counters[i+4] -= hash & 1; hash >>= 1;
        bloom->counters[i+5] -= hash & 1; hash >>= 1;
        bloom->counters[i+6] -= hash & 1; hash >>= 1;
        bloom->counters[i+7] -= hash & 1; hash >>= 1;
        mask |= !!bloom->counters[i+0] << (i  );
        mask |= !!bloom->counters[i+1] << (i+1);
        mask |= !!bloom->counters[i+2] << (i+2);
        mask |= !!bloom->counters[i+3] << (i+3);
        mask |= !!bloom->counters[i+4] << (i+4);
        mask |= !!bloom->counters[i+5] << (i+5);
        mask |= !!bloom->counters[i+6] << (i+6);
        mask |= !!bloom->counters[i+7] << (i+7);
    }
    bloom->bitmap = bloom->bitmap & mask;
}

constexpr bool includedb__bloomMaybehave(includedb__bloom *bloom, includedb__inttype hash)
{
    return !((hash&bloom->bitmap) ^ hash);
}


/*++++++++++++++++++++++++++++++++++++++*/
/*                                Tests */
/*++++++++++++++++++++++++++++++++++++++*/
#if !defined(DISABLE_TESTS)

constexpr unsigned includedb_testBloom()
{
    includedb__bloom bloomTest;

    includedb__bloomPut(&bloomTest, 0b0110);
    const bool TEST_PUT_OK = includedb__bloomMaybehave(&bloomTest, 0b0110);
    const bool TEST_SURE_DOESNT_HAVE = includedb__bloomMaybehave(&bloomTest, 0b0001) == false;
    const bool TEST_OVERLAPPING_BITS = includedb__bloomMaybehave(&bloomTest, 0b0100) == true;
    const bool TEST_CONFLICTING_BITS = includedb__bloomMaybehave(&bloomTest, 0b1100) == false;
    includedb__bloomPut(&bloomTest, 0b1111);
    includedb__bloomRemove(&bloomTest, 0b0110);
    const bool TEST_KEY_FOUND_AFTER_REMOVE = includedb__bloomMaybehave(&bloomTest, 0b0110) == true;
    includedb__bloomRemove(&bloomTest, 0b1111);
    const bool TEST_KEY_NOT_FOUND_AFTER_REMOVE = includedb__bloomMaybehave(&bloomTest, 0b1111) == false;    

    return TEST_PUT_OK
        | (TEST_SURE_DOESNT_HAVE << 1)
        | (TEST_OVERLAPPING_BITS << 2)
        | (TEST_CONFLICTING_BITS << 3)
        | (TEST_KEY_FOUND_AFTER_REMOVE << 4)
        | (TEST_KEY_NOT_FOUND_AFTER_REMOVE << 5)
        ;
}
static constexpr unsigned resBloomtest = includedb_testBloom();

static_assert(resBloomtest& 1, "Didn't work");
static_assert(resBloomtest& 2, ".maybeHave() returned 'true' when the key clearly didn't exist");
static_assert(resBloomtest& 4, ".maybeHave() overlapping bits");
static_assert(resBloomtest& 8, ".maybeHave() conflicting bits");
static_assert(resBloomtest&16, "key disappeard after remove");
static_assert(resBloomtest&32, "key should have disappeared after remove");

#endif // !defined(DISABLE_TESTS)

