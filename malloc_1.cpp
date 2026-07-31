#include <unistd.h>
#include <iostream>
#include <cstring>

const size_t MAX = 100000000;

void* smalloc(size_t size)
{
    if(size > MAX)
    {
        return NULL;
    }
    if(size == 0)
    {
        return NULL;
    }

    void* ret_val = sbrk(size);
    if(ret_val == (void*)(-1))
    {
        return NULL;
    }
    return ret_val;
}