#include "window.h"
#include "tree_render.h"
#include "ds/list.h"
#include "ds/node.h"
#include "ds/bt.h"
#include "context.h"
#include "layout.h"

#include <assert.h>

#define NODE_RAD 25
#define NODE_DIM 50
#define FONT_SIZE 30

static ImDrawList* dl;
static LayoutNode* rootLayout;
static UIContext* ctx;

void R_Init() {
	Arena* arena = malloc(sizeof(Arena));
	*arena = (Arena){ malloc(64 * 1024), 64 * 1024, 0 };

	UIStateMap* map = malloc(sizeof(UIStateMap));
	*map = (UIStateMap){ calloc(64, sizeof(NodeUIState)), 64 };

	ctx = malloc(sizeof(UIContext));
	*ctx = (UIContext){
		.frameArena  = arena,
		.uiStateMap  = map,
		.drawCmds    = malloc(1024 * sizeof(DrawCmd)),
		.drawCmdCap  = 1024,
	};
}

void R_DrawFilledCircle(const ImVec2_c pos, const uint16_t radius, const uint32_t col) {
	ImDrawList_AddCircleFilled(dl, pos, radius, col, 0);
}

void R_DrawNumber(const ImVec2_c pos, const int num, const uint32_t col) {
	char buf[6];
	snprintf(buf, sizeof(buf), "%d", num);

	ImVec2_c size = igCalcTextSize(buf, NULL, false, 0);

	ImVec2_c endPos = { pos.x - size.x, pos.y - size.y };

	ImDrawList_AddText_FontPtr(dl, igGetFont(), FONT_SIZE, endPos, col, buf, NULL, 0.0f, NULL);
}

void R_RenderNode(const struct Node* node, struct ImVec2_c pos, const uint32_t col) {
	R_DrawFilledCircle(pos, NODE_RAD, col);
	R_DrawNumber(pos, node->val, col - 0x00FFFFFF);
}

bool R_TreeWidget(const Node* node, float x, float y) {
	NodeUIState* st = CTX_UIGetOrCreate(ctx->uiStateMap, node->id);

	float w = NODE_RAD, h = NODE_RAD;
	bool hovered = ctx->mouseX >= x - w && ctx->mouseX <= x + w &&
		ctx->mouseY >= y - h && ctx->mouseY <= y + h;


	if (hovered) ctx->hotId = node->id;
	bool clicked = false;
	if (hovered && ctx->mousePressed) ctx->activeId = node->id;
	if (ctx->activeId == node->id && ctx->mouseReleased) {
		clicked = hovered; // released while still over it
		ctx->activeId = 0;
	}

	uint32_t color = (ctx->activeId == node->id) ? 0xFFFF0000 :
		(ctx->hotId == node->id) ? 0xFFFFFF00 : 0xFFFFFFFF;

	CTX_AddCmd(ctx, (DrawCmd) { CMD_CIRCLE, x, y, w, 0, color, NULL });
	CTX_AddCmd(ctx, (DrawCmd) { CMD_TEXT, x + 4, y + 4, 0, 0, 0xFFFFFFFF, "some_val" });

	if (clicked) st->expanded = !st->expanded;
	st->x = x;
	st->y = y;
	st->id = node->id;

	return clicked;
}

void R_RenderConnectingLine(const struct Node* node, struct ImVec2_c nodePos, int dep) {
	float sign = (node->par->children[0] == node ? 1 : -1);
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

void R_RenderTree(const BT* tree) {
	dl = igGetForegroundDrawList_ViewportPtr(NULL);

	static uint32_t col = 0xFFFFFFFF;

	struct ImVec2_c pos = { 300, 200 };

	size_t index = 0;
	size_t maxDepth = BT_MaxDepth(tree);
	size_t treeHeight = maxDepth * NODE_DIM;

	pos.y += treeHeight;

	AQueue depth = AQ_Init(tree->size);
	FlQueue path = FlQueue_Init(tree->size);
	NQueue queue = NQueue_Init(tree->size);

	NQueue_Enqueue(tree->root, &queue);
	FlQueue_Enqueue(pos.x, &path);
	AQ_Enqueue(maxDepth, &depth);

	while (!NQueue_IsEmpty(&queue)) {
		struct Node* node = NQueue_Dequeue(&queue);
		float x = FlQueue_Dequeue(&path);
		int dep = AQ_Dequeue(&depth);

		if (node->children[0]) {
			NQueue_Enqueue(node->children[0], &queue);
			FlQueue_Enqueue(x - (float)dep * NODE_DIM, &path);
			AQ_Enqueue(dep - 1, &depth);
		}
		if (node->children[1]) {
			NQueue_Enqueue(node->children[1], &queue);
			FlQueue_Enqueue(x + (float)dep * NODE_DIM, &path);
			AQ_Enqueue(dep - 1, &depth);
		}

		struct ImVec2_c endPos = { x, pos.y - dep * NODE_DIM };

		if (node->par) {
			R_RenderConnectingLine(node, endPos, dep);
		}

		R_RenderNode(node, endPos, col);
	}
}

void R_EmitWidgets(Node* root, LayoutNode* layout) {
	if (!root || !layout) return;

	if (!layout->children[0] && !layout->children[1]) {
		R_TreeWidget(root, layout->x, layout->y);
		return;
	}

	for (int i = 0; i < root->childCount; i++) {
		R_EmitWidgets(root->children[i], layout->children[i]);
	}

}

void R_BackendRender() {
	for (int i = 0; i < ctx->drawCmdCount; i++) {
		CMD_ProcessCommand(&ctx->drawCmds[i]);
	}
}

void R_TreeFrame(Node* root) {
	ImGuiIO* io = igGetIO();
	ctx->mouseX        = io->MousePos.x;
	ctx->mouseY        = io->MousePos.y;
	ctx->mouseDown     = io->MouseDown[0];
	ctx->mousePressed  = io->MouseClicked[0];
	ctx->mouseReleased = io->MouseReleased[0];

	A_Reset(ctx->frameArena);

	ctx->drawCmdCount = 0;
	ctx->hotId = 0;

	float cursorX = 0;
	LayoutNode* node = A_Alloc(ctx->frameArena, sizeof(LayoutNode));
	L_LayoutTree(root, 0, &cursorX, ctx, node);

	R_EmitWidgets(root, node);

	R_BackendRender();
}