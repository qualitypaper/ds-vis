#ifndef GRAPH_VIS_NODE
#define GRAPH_VIS_NODE

#include "defs.h"
#include <stdlib.h>

#define IsLeftChild(node) ((node->par->children[0]) == node)
#define IsRightChild(node) ((node->par->children[1]) == node)

#define node_left(n)	((n)->children[0])
#define node_right(n)	((n)->children[1])
#define node_sibling(n) (IsLeftChild(n) ? node_right(n->par) : node_left(n->par))

typedef struct Node {
	ID id;
	struct Node** children;
	size_t        childCount;
	struct Node* par;
	int           val;
	int balance; // for AVL trees
	int height;  // for AVL trees
	COLOR color;
} Node;

Node* Node_Alloc(int val, size_t slots);
void Node_Free(Node* n);

#endif
