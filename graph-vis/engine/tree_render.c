#include "window.h"
#include "tree_render.h"
#include "ds/bt.h"
#include "ds/list.h"

#define NODE_RAD 25
#define NODE_DIM 50
#define FONT_SIZE 30

DEFINE_LIST(struct RNode*, RNList);

struct RNList nodes;
ImDrawList* dl;

void R_Init() {
	nodes = RNList_Init(2);
}

void R_DrawFilledCircle(const struct ImVec2_c pos, const uint16_t radius, const uint32_t col) {
	ImDrawList_AddCircleFilled(dl, pos, radius, col, 0);
}

void R_DrawNumber(const struct ImVec2_c pos, const int num, const uint32_t col) {
	char buf[6];
	snprintf(buf, sizeof(buf), "%d", num);

	struct ImVec2_c size = igCalcTextSize(buf, NULL, false, 0);

	struct ImVec2_c endPos = { pos.x - size.x, pos.y - size.y };

	ImDrawList_AddText_FontPtr(dl, igGetFont(), FONT_SIZE, endPos, col, buf, NULL, 0.0f, NULL);
}

void R_RenderNode(const struct Node* node, struct ImVec2_c pos, const uint32_t col) {
	R_DrawFilledCircle(pos, NODE_RAD, col);
	R_DrawNumber(pos, node->val, col - 0x00FFFFFF);
}

void R_RenderConnectingLine(const struct Node* node, struct ImVec2_c nodePos, int dep) {
	int sign = (node->par->left == node ? 1 : -1);
	// parent x = child x ± (dep+1)*NODE_DIM, since child was placed at parent_x ∓ dep*NODE_DIM when parent was at dep+1
	struct ImVec2_c prevPos = {
		nodePos.x + (dep + 1) * NODE_DIM * sign,
		nodePos.y - NODE_DIM
	};

	float dx = abs(nodePos.x - prevPos.x);
	float dy = abs(nodePos.y - prevPos.y);
	float c = sqrtf(dx * dx + dy * dy);

	float sn = dx / c;
	float cs = dy / c;

	prevPos.x -= NODE_RAD * sn * sign;
	prevPos.y += NODE_RAD * cs;

	ImDrawList_AddLine(dl, prevPos, nodePos, 0xFFCFCFFF, 3);
}

void R_RenderTree(const struct BT* tree) {
	dl = igGetForegroundDrawList_ViewportPtr(NULL);

	static uint32_t col = 0xFFFFFFFF;

	struct ImVec2_c pos = { 300, 200 };

	size_t index = 0;
	size_t maxDepth = BT_MaxDepth(tree);
	size_t treeHeight = maxDepth * NODE_DIM;

	pos.y += treeHeight;

	struct AQueue depth = AQ_Init(tree->size);
	struct FlQueue path = FlQueue_Init(tree->size);
	struct NQueue queue = NQueue_Init(tree->size);

	NQueue_Enqueue(tree->root, &queue);
	FlQueue_Enqueue(pos.x, &path);
	AQ_Enqueue(maxDepth, &depth);

	while (!NQueue_IsEmpty(&queue)) {
		struct Node* node = NQueue_Dequeue(&queue);
		float x = FlQueue_Dequeue(&path);
		int dep = AQ_Dequeue(&depth);

		if (node->left) {
			NQueue_Enqueue(node->left, &queue);
			FlQueue_Enqueue(x - dep * NODE_DIM, &path);
			AQ_Enqueue(dep - 1, &depth);
		}
		if (node->right) {
			NQueue_Enqueue(node->right, &queue);
			FlQueue_Enqueue(x + dep * NODE_DIM, &path);
			AQ_Enqueue(dep - 1, &depth);
		}

		struct ImVec2_c endPos = { x, pos.y - dep * NODE_DIM };

		if (node->par) {
			R_RenderConnectingLine(node, endPos, dep);
		}

		R_RenderNode(node, endPos, col);
		struct RNode* rn = malloc(sizeof(struct RNode));

		rn->id = index++;
		rn->node = node;
		rn->left = NULL;
		rn->right = NULL;
		rn->pos = endPos;
		rn->col = col;

		RNList_Add(rn, &nodes);
	}
}
