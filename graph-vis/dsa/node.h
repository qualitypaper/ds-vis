#ifndef GRAPH_VIS_NODE
#define GRAPH_VIS_NODE

#include "base/core.h"
#include "base/arena.h"

typedef struct Node Node;
struct Node {
	ID id;
	Node** children;
	U64        childCount;
	Node* par;
	S32           val;
	S32 balance; // for AVL trees
	S32 height;  // for AVL trees
	COLOR color;
};

Node* node_alloc(Arena* arena, S32 val, U64 slots);

#define IsLeftChild(node) ((node->par->children[0]) == node)
#define IsRightChild(node) ((node->par->children[1]) == node)

#define NodeLeft(n)	((n)->children[0])
#define NodeRight(n)	((n)->children[1])
#define NodeSibling(n) (IsLeftChild(n) ? NodeRight(n->par) : NodeLeft(n->par))

#endif
