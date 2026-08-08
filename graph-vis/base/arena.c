#include "base/arena.h"

Arena* arena_alloc(U64 size)
{
    Arena* a = malloc(sizeof(Arena));
    a->base = malloc(size);
    a->size = size;
    a->used = 0;
    a->next = NULL;
    a->prev = NULL;
    return a;
}

Arena* arena_alloc_next(Arena* a, U64 capacity)
{
    Arena* next = malloc(sizeof(Arena));
    next->base = malloc(capacity);
    next->size = capacity;
    next->used = 0;
    next->next = NULL;
    next->prev = a;
    a->next = next;
    return next;
}

void* arena_push(Arena* a, U64 n, U64 alignment)
{
    n = (n + (alignment - 1)) & ~(alignment - 1);

    if (a->used + n > a->size)
    {
        Arena* temp = a;
        if (a->next)
        {
            temp = a->next;
        }
        else
        {
            // allocate next arena, big enough for the pending push (a single
            // block-sized allocation must always fit somewhere in the chain)
            temp = arena_alloc_next(a, Max(a->size, n));
        }
        return arena_push(temp, n, alignment);
    }

    void* p = a->base + a->used;
    a->used += n;
    return p;
}

void arena_reset(Arena* a)
{
    /* reset the whole chain so overflow arenas are reused, not leaked */
    for (Arena* cur = a; cur; cur = cur->next) cur->used = 0;
}

void arena_free(Arena* a)
{
    if (a->prev)
    {
        a->prev->next = NULL;
    }
    /* free the whole chain from a onward */
    while (a)
    {
        Arena* next = a->next;
        free(a->base);
        free(a);
        a = next;
    }
}
