#ifndef GRAPH_VIS_ARENA
#define GRAPH_VIS_ARENA
#include <stdint.h>

typedef struct Arena {
	uint8_t* base;
	uint64_t size;
	uint64_t used;
} Arena;

void* A_Alloc(Arena* a, uint64_t n);
void A_Reset(Arena* a);

#endif