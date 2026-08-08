#ifndef GRAPH_VIS_ARENA
#define GRAPH_VIS_ARENA
#include "base/core.h"

typedef struct Arena Arena;
struct Arena {
    Arena* next;
    Arena* prev;
    U8* base;
	U64 size;
	U64 used;
};

Arena* arena_alloc(U64 size);
Arena* arena_alloc_next(Arena* a, U64 capacity);

void* arena_push(Arena* a, U64 n, U64 alignment);
void arena_reset(Arena* a);
void arena_free(Arena* a);

#define PushStruct(a, T) ((T*) (arena_push(a, sizeof(T), AlignOf(T))))

#define PushArrayNoZeroAligned(a, T, c, align) (T *)arena_push((a), sizeof(T)*(c), (align))
#define PushArrayAligned(a, T, c, align) (T *)arena_push((a), sizeof(T)*(c), (align))
#define PushArrayNoZero(a, T, c) PushArrayNoZeroAligned(a, T, c, Max(8, AlignOf(T)))
#define PushArray(a, T, c) PushArrayAligned(a, T, c, Max(8, AlignOf(T)))

#endif
