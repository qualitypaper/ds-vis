#ifndef GRAPH_VIS_BT
#define GRAPH_VIS_BT

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "queue.h"

DEFINE_LIST(BNode*, NList)
DEFINE_QUEUE(BNode*, NQueue)

typedef enum TreeType
{
    BINARY, BST, RED_BLACK, AVL
} TreeType;

typedef struct BT
{
    TreeType type;
    BNode* root;
    BNode* sentinel;
    U64 size;
    Arena* arena; // owns all nodes; reset on rebuild, freed at exit
} BT;

// helper macros
#define IsValid(n) (n != NULL && (n) != (n->par))


// bt
BNode* BT_Build(Arena* arena, int preorder[], int inorder[], int ilower, int iupper, int index);
BT BT_Init(S32Array preorder, S32Array inorder);
size_t BT_MaxDepth(BT* tree);
void BT_Add(int val, BT* tree);
void BT_Transplant(BNode* root, BNode* node, BNode* newNode);
void BT_Del(BNode* node, BT* tree);
void BT_DelByVal(int val, BT* tree);

// bst
BNode* BST_Build(Arena* arena, int preorder[], int index, int upper);
BT BST_Init(int preorder[], size_t len);
BNode* BST_Search(int val, BT* bt);
void BST_Add(int val, BT* bt);
void BST_Del(BNode* node, BT* bt);

// rb
BNode* RB_BNodeAlloc(Arena* treeArena, int val, BNode* sentinel);
BT RB_Init(int preorder[], size_t len);

void RB2AVL(BT* rb);
void RB2BST(BT* bt);

void RB_LeftRotate(BNode* node, BT* bt);
void RB_RightRotate(BNode* node, BT* bt);
void RB_Transplant(BNode* oldNode, BNode* newNode, BT* bt);

void RB_AddFixup(BNode* node, BT* bt);
void RB_Add(int val, BT* bt);

void RB_DelFixup(BNode* fixupNode, BNode* parent, BT* bt);
void RB_Del(BNode* node, BT* bt);


// avl
void AVL_LeftRotate(BNode* node, BT* bt);
void AVL_RightRotate(BNode* node, BT* bt);

BT AVL_Init(int preorder[], size_t len);

void AVL2RB(BT* bt);
void AVL2BST(BT* bt);

void AVL_AddFixup(BNode* inserted, BT* bt);
void AVL_Add(int val, BT* bt);
void AVL_DelFixup(BNode* node, BT* bt);
void AVL_Del(BNode* node, BT* bt);


// transformations
void BST2RB(BT* bt);
void BST2AVL(BT* bt);

#endif
