#ifndef GRAPH_VIS_NODE
#define GRAPH_VIS_NODE

#include "base/core.h"
#include "base/arena.h"

typedef struct BNode BNode;

struct BNode
{
    ID id;
    BNode* left;
    BNode* right;
    BNode* par;
    S32 val;
    S32 balance; // for AVL trees
    S32 height; // for AVL trees
    COLOR color;
};

BNode* bnode_alloc_nil(Arena* arena, S32 val, BNode* nil);
#define bnode_alloc(arena, val) (bnode_alloc_nil(arena, val, NULL))

#define BLeft(n)           ((n)->left)
#define BRight(n)          ((n)->right)
#define BIsLeftChild(node)  ((node)->par->left == (node))
#define BIsRightChild(node) ((node)->par->right == (node))

#endif
