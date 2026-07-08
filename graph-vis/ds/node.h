#ifndef GRAPH_VIS_NODE
#define GRAPH_VIS_NODE

#include "defs.h"
#include <stdlib.h>

typedef struct Node {
	ID id;
	struct Node** children;
	size_t        childCount;
	struct Node* par;
	int           val;
} Node;

static inline Node* Node_Alloc(int val, size_t slots) {
	Node* n = (Node*)malloc(sizeof(Node));
	n->children = (Node**)calloc(slots, sizeof(Node*));
	n->childCount = slots;
	n->par = NULL;
	n->val = val;
	n->id  = (ID)n; // ponytail: pointer-as-id, unique per allocation
	return n;
}

static inline void Node_Free(Node* n) {
	free(n->children);
	free(n);
}

#endif
