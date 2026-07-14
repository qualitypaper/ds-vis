#include "node.h"


Node* Node_Alloc(int val, size_t slots) {
	Node* n = (Node*)malloc(sizeof(Node));
	n->children = (Node**)calloc(slots, sizeof(Node*));
	n->childCount = slots;
	n->par = NULL;
	n->val = val;
	n->id = (ID)n; // ponytail: pointer-as-id, unique per allocation
	n->color = FOREGROUND;
	return n;
}

void Node_Free(Node* n) {
	free(n->children);
	free(n);
}