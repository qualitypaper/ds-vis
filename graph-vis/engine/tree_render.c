#include "window.h"
#include "tree_render.h"
#include "ds/bt.h"
#include "ds/list.h"

#define NODE_RAD 25
#define FONT_SIZE 30

DEFINE_LIST(struct BT*, BTList);
DEFINE_QUEUE(struct Node*, NodeQueue);

struct BTList bts;

void R_Init() {
	bts = BTList_Init(2);
}

void R_DrawFilledCircle(const struct ImVec2_c pos, const uint16_t radius, const uint32_t col) {

	ImDrawList* dl = igGetForegroundDrawList_ViewportPtr(NULL);

	ImDrawList_AddCircleFilled(dl, pos, radius, 0xffffffff, 0);
}

void R_DrawNumber(const struct ImVec2_c pos, const int num, const uint32_t col) {
	char buf[6];
	snprintf(buf, sizeof(buf), "%d", num);
	ImDrawList* dl = igGetForegroundDrawList_ViewportPtr(NULL);

	struct ImVec2_c endPos = { pos.x - FONT_SIZE / 2, pos.y - FONT_SIZE / 2 };

	ImDrawList_AddText_FontPtr(dl, igGetFont(), FONT_SIZE, endPos, col, buf, NULL, 0.0f, NULL);
}

void R_RenderNode(const struct Node* node, struct ImVec2_c pos, const uint32_t col) {
	R_DrawFilledCircle(pos, NODE_RAD, col);
	R_DrawNumber(pos, node->val, col - 0x00FFFFFF);
}

void R_RenderTree(const struct BT* tree) {
	struct ImVec2_c pos = { 300, 100 };

	struct AQueue depth = AQ_Init(tree->size / 2);
	struct AQueue path = AQ_Init(tree->size / 2);
	struct NodeQueue queue = NodeQueue_Init(tree->size);

	NodeQueue_Enqueue(tree->root, &queue);
	AQ_Enqueue(0, & path);
	AQ_Enqueue(0, &depth);

	while (!NodeQueue_IsEmpty(&queue)) {
		struct Node* node = NodeQueue_Dequeue(&queue);
		int dir = AQ_Dequeue(&path);
		int dep = AQ_Dequeue(&depth);

		if (node->left) {
			NodeQueue_Enqueue(node->left, &queue);
			AQ_Enqueue(-1, &path);
			AQ_Enqueue(dep + 1, &depth);
		}
		if (node->right) {
			NodeQueue_Enqueue(node->right, &queue);
			AQ_Enqueue(1, &path);
			AQ_Enqueue(dep + 1, &depth);
		}

		int mul = dir * dep;
		struct ImVec2_c endPos = { pos.x + dir * NODE_RAD + mul * NODE_RAD/5, pos.y + dep * 2 * (NODE_RAD)};

		R_RenderNode(node, endPos, 0xFFFFFFFF);
	}
}
