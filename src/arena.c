#include "arena.h"

void *arena_alloc(arena *arena, int byte_size)
{
    if (!arena)
    {
        DEBUG_BREAK;
    }
    if (byte_size % 2 != 0)
    {
        DEBUG_BREAK;
    }
    void *ret_mem = NULL;
    if (byte_size + arena->allocated_till_now < arena->total_size)
    {
        ret_mem                    = arena->ptr;
        arena->ptr                += byte_size;
        arena->allocated_till_now += byte_size;
    }
    else
    {
        printf("Arena doesn't have enought space to accodomate allocation of size %d bytes. Arena total size %d, Arena "
               "allocated size till now %d",
               byte_size, arena->total_size, arena->allocated_till_now);
    }

    if (ret_mem == NULL)
    {
        DEBUG_BREAK;
    }

    memset(ret_mem, 0, byte_size);
    return ret_mem;
}

arena arena_init(void *mem, int total_size)
{
    if (!mem)
    {
        DEBUG_BREAK;
    }
    arena arena              = {};
    arena.mem                = mem;
    arena.ptr                = mem;
    arena.total_size         = total_size;
    arena.allocated_till_now = 0;

    return arena;
}

void arena_reset(arena *arena)
{
    arena->allocated_till_now = 0;
    arena->ptr                = arena->mem;
    memset(arena->mem, 0, arena->total_size);
}

