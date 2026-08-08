#ifndef GRAPH_VIS_TREE_RENDER
#define GRAPH_VIS_TREE_RENDER

#include "base/core.h"
#include "dsa/bt.h"
#include "engine/layout.h"

void render_init();
S32 tree_widget(const BNode* node, F32 x, F32 y, BT* bt);
void add_connecting_line(LayoutTreeNode* layout, int childIdx);

void emit_widgets(BNode* root, LayoutTreeNode* layout, BT* bt);
void backend_render();
void emit_control_widgets(BT* bt);
void emit_tree_type_popup(BT* bt);

void tree_frame(BT* tree, F32 x, F32 y);

#endif
