#include "includedb_config.h"


static constexpr int includedb__dbBufferResize(includeDB *instance, int newsize)
{
    const int sz = instance->szBuf;
    if (sz < newsize)
    {
        instance->szBuf = ((newsize/sz) * sz) + sz;
        unsigned char *tmp = (unsigned char *)includedb__realloc(instance->buffer, instance->szBuf);
        if (!tmp)
        {
            instance->ec = INCLUDEDB__BUFFER_ALLOC;
            return includedb__error;
        }
        instance->buffer = tmp;
    }
    return includedb__ok;
}

static int includedb__dbWrite(includeDB *instance, int location, int amount)
{
    location += 128; // db header size
    const int res = includedb__fileWrite(&instance->file, instance->buffer, amount, location);
    return res != amount;
}

static int includedb__dbRead(includeDB *instance, int location, int amount)
{
    location += 128;
    
    // make sure the buffer is big enough:
    if (includedb__dbBufferResize(instance, amount) == includedb__error)
        return includedb__error;
    
    includedb__fileRead(&instance->file, instance->buffer, amount, location);
    return includedb__ok;
}

