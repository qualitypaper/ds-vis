#ifndef GRAPH_VIS_TREE_RENDER
#define GRAPH_VIS_TREE_RENDER
struct BT;

struct RNode {
	uint32_t id;
	struct Node* node;
	struct RNode* left;
	struct RNode* right;

	struct ImVec2_c pos;
	uint32_t col;
};

void R_Init();
void R_DrawNumber(const struct ImVec2_c pos, const int num, const uint32_t col);
void R_DrawFilledCircle(const ImVec2_c pos, const uint16_t radius, const uint32_t col);
void R_RenderNode(const struct Node* node, struct ImVec2_c pos, const uint32_t col);
void R_RenderConnectingLine(const struct Node* node, ImVec2_c nodePos, int dep);
void R_RenderTree(const struct BT* tree);

#endif