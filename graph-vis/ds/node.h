#ifndef GRAPH_VIS_NODE
#define GRAPH_VIS_NODE

#include "defs.h"
#include <stdlib.h>

#define node_left(n)	((n)->children[0])
#define node_right(n)	((n)->children[1])

typedef struct Node {
	ID id;
	struct Node** children;
	size_t        childCount;
	struct Node* par;
	int           val;
	COLOR color;
} Node;

Node* Node_Alloc(int val, size_t slots);
void Node_Free(Node* n);

#endif
