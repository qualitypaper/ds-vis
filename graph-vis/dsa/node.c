#include "node.h"

BNode* bnode_alloc_nil(Arena* arena, S32 val, BNode* nil)
{
    BNode* n = PushStruct(arena, BNode);
    n->left = nil;
    n->right = nil;
    n->par = nil;
    n->val = val;
    n->id = (ID)n; // pointer-as-id, unique per allocation
    n->color = FOREGROUND;
    n->balance = 0;
    n->height = 0;
    return n;
}
