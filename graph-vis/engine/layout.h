#ifndef GRAPH_VIS_LAYOUT
#define GRAPH_VIS_LAYOUT

#include "dsa/node.h"
#include "context.h"

typedef struct LayoutNode LayoutNode;

struct LayoutNode {
	Node*              node;
	F32              x, y, subtreeWidth;
	LayoutNode* children[2];
};

F32 layout_tree(Node *n, F32 depthY, F32 *cursorX, UIContext* ctx, LayoutNode *out);

#endif
