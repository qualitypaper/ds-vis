#ifndef GRAPH_VIS_TREE_RENDER
#define GRAPH_VIS_TREE_RENDER
struct BT;

void R_Init();
void R_DrawNumber(const struct ImVec2_c pos, const int num, const uint32_t col);
void R_DrawFilledCircle(const ImVec2_c pos, const uint16_t radius, const uint32_t col);
void R_RenderNode(const struct Node* node, struct ImVec2_c pos, const uint32_t col);
void R_RenderTree(const struct BT* tree);

#endif