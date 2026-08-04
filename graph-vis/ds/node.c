#include "node.h"


Node* Node_Alloc(int val, size_t slots) {
	Node* n = (Node*)malloc(sizeof(Node));
	n->children = (Node**)calloc(slots, sizeof(Node*));
	n->childCount = slots;
	n->par = NULL;
	n->val = val;
	n->id = (ID)n; // pointer-as-id, unique per allocation
	n->color = FOREGROUND;
	n->balance = 0;
	n->height = 0;
	return n;
}

void Node_Free(Node* n) {
	free(n->children);
	free(n);
}