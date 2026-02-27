#pragma once
#include "defines.h"

typedef struct arena
{
    int total_size;
    int allocated_till_now;
    // the index
    void *ptr;
    // starting mem
    void *mem;
} arena;

void *arena_alloc(arena *arena, int byte_size);
arena arena_init(void *mem, int total_size);
void  arena_reset(arena *arena);
