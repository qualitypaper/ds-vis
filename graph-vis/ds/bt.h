#ifndef GRAPH_VIS_BT
#define GRAPH_VIS_BT

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "queue.h"

DEFINE_LIST(Node*, NList)
DEFINE_QUEUE(Node*, NQueue)

typedef enum TreeType {
	BINARY, BST, RED_BLACK, AVL
} TreeType;

typedef struct BT {
	TreeType type;
	Node* root;
	size_t size;
} BT;


// bt
Node* BT_Build(int preorder[], int inorder[], int ilower, int iupper, int index);
BT       BT_Init(int preorder[], int inorder[], size_t len);
size_t   BT_MaxDepth(BT* tree);
void     BT_Add(int val, BT* tree);
void     BT_Transplant(Node* root, Node* node, Node* newNode);
void     BT_Del(Node* node, BT* tree);
void     BT_DelByVal(int val, BT* tree);

// bst
Node* BST_Build(int preorder[], int index, int upper);
BT       BST_Init(int preorder[], size_t len);
Node* BST_Search(int val, BT* bt);
void     BST_Add(int val, BT* bt);
void     BST_Del(Node* node, BT* bt);

// rb
Node* RB_NodeAlloc(int val);
BT RB_Init(int preorder[], size_t len);
void RB_LeftRotate(Node* node, BT* bt);
void RB_AddFixup(Node* node, BT* bt);
void RB_RightRotate(Node* node, BT* bt);
void RB_DelFixup(Node* fixupNode, Node* parent, BT* bt);
void RB_Transplant(Node* oldNode, Node* newNode, BT* bt);
void RB_Add(int val, BT* bt);
void RB_Del(Node* node, BT* bt);

// avl
int AVL_GetBalance(Node* root);
int8_t AVL_CheckAndUpdateBalance(Node* node);
void AVL_UpdateHeight(Node * node);
BT AVL_Init(int preorder[], size_t len);
void AVL2RB(BT* bt);

void AVL_AddFixup(Node* node, int addedValue, BT* bt);
void AVL_Add(int val, BT* bt);
void AVL_DelFixup(Node* node, BT* bt);
void AVL_Del(Node* node, BT* bt);

uint16_t BT_Depth(const Node* root);

#endif
