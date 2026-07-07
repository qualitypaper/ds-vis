#ifndef GRAPH_VIS_BT
#define GRAPH_VIS_BT

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "queue.h"

DEFINE_LIST(Node*, NList)
DEFINE_QUEUE(Node*, NQueue)

typedef struct BT {
	Node* root;
	size_t size;
} BT;

Node* BT_Build(int preorder[], int inorder[], int ilower, int iupper, int index);
BT       BT_Init(int preorder[], int inorder[], size_t len);
size_t   BT_MaxDepth(BT* tree);
void     BT_Add(int val, BT* tree);
void     BT_Transplant(Node* root, Node* node, Node* newNode);
void     BT_Del(Node* node, BT* tree);
void     BT_DelByVal(int val, BT* tree);

Node* BST_Build(int preorder[], size_t index, size_t upper);
BT       BST_Init(int preorder[], size_t len);
Node* BST_Search(int val, BT* bt);
void     BST_Add(int val, BT* bt);
void     BST_Del(Node* node, BT* bt);

uint16_t BT_Depth(const Node* root);
void     BT_Print(const BT* bt);

#endif
