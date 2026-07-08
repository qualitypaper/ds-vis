#ifndef GRAPH_VIS_LAYOUT
#define GRAPH_VIS_LAYOUT

#include "ds/node.h"
#include "context.h"

typedef struct LayoutNode {
	Node*              node;
	float              x, y, subtreeWidth;
	struct LayoutNode* children[2];
} LayoutNode;

float L_LayoutTree(Node *n, float depthY, float *cursorX, UIContext* ctx, LayoutNode *out);

#endif