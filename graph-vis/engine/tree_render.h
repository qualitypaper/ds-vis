#ifndef GRAPH_VIS_TREE_RENDER
#define GRAPH_VIS_TREE_RENDER

struct ImVec2_c;
struct BT;
struct Node;
struct UIContext;
struct LayoutNode;

void R_Init();
int R_TreeWidget(const struct Node* node, float x, float y);
void R_AddConnectingLine(struct LayoutNode* layout, int childIdx);

void R_EmitWidgets(struct Node* root, struct LayoutNode* layout);
void R_BackendRender();
void R_EmitControlWidgets(struct BT* bt);
void R_EmitNodePopup(struct BT* bt);
void R_TreeFrame(struct BT* tree, float x, float y);

#endif