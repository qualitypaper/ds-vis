#ifndef GRAPH_VIS_LAYOUT
#define GRAPH_VIS_LAYOUT

#include "context.h"

typedef struct LayoutTreeNode LayoutTreeNode;

struct LayoutTreeNode
{
    BNode* node;
    F32 x, y, subtreeWidth;
    LayoutTreeNode* children[2];
};

void layout_tree(BNode* n, F32 depthY, F32 cursorX, UIContext* ctx, LayoutTreeNode* out);

#endif
