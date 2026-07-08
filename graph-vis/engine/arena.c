#include "arena.h"
#include <assert.h>

void* A_Alloc(Arena* a, size_t n)
{
	// 16-byte align
	n = (n + 15) & ~15;
	assert(a->used + n <= a->size);

	void* p = a->base + a->used;
	a->used += n;
	return p;
}

void A_Reset(Arena* a) {
	a->used = 0;
}
