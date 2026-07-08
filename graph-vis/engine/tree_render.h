#ifndef GRAPH_VIS_TREE_RENDER
#define GRAPH_VIS_TREE_RENDER

struct ImVec2_c;
struct BT;
struct Node;
struct UIContext;
struct LayoutNode;

void R_Init();
void R_DrawFilledCircle(const struct ImVec2_c pos, const uint16_t radius, const uint32_t col);
bool R_TreeWidget(const struct Node* node, float x, float y);
void R_AddConnectingLine(struct LayoutNode* layout, int childIdx);

void R_EmitWidgets(struct Node* root, struct LayoutNode* layout);
void R_BackendRender();
void R_TreeFrame(struct Node* root, float x, float y);

#endif