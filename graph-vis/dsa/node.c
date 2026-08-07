#include "node.h"

Node* node_alloc(Arena* arena, S32 val, U64 slots) {
	Node* n = PushStruct(arena, Node);
	n->children = PushArray(arena, Node*, slots);
	n->childCount = slots;
	n->par = NULL;
	n->val = val;
	n->id = (ID)n; // pointer-as-id, unique per allocation
	n->color = FOREGROUND;
	n->balance = 0;
	n->height = 0;
	MemorySet(n->children, 0, slots * sizeof(Node*)); // arena memory is not zeroed
	return n;
}
