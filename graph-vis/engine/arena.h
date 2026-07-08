#ifndef GRAPH_VIS_ARENA
#define GRAPH_VIS_ARENA
#include <stdint.h>

typedef struct Arena {
	uint8_t* base;
	size_t size;
	size_t used;
} Arena;

void* A_Alloc(Arena* a, size_t n);
void A_Reset(Arena* a);

#endif