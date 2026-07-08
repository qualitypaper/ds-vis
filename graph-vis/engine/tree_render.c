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
		.frameArena = arena,
		.uiStateMap = map,
		.drawCmds = malloc(1024 * sizeof(DrawCmd)),
		.drawCmdCap = 1024,
	};
}

void R_DrawFilledCircle(const ImVec2_c pos, const uint16_t radius, const uint32_t col) {
	ImDrawList_AddCircleFilled(dl, pos, radius, col, 0);
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
	CTX_AddCmd(ctx, (DrawCmd) { CMD_NUM, x, y, 0, 0, 0xFF000000, node->val });

	if (clicked) st->expanded = !st->expanded;
	st->x = x;
	st->y = y;
	st->id = node->id;

	return clicked;
}

void R_AddConnectingLine(LayoutNode* layout, int childIdx) {
	LayoutNode* child = layout->children[childIdx];
	if (!child) return;

	float dx = fabsf(child->x - layout->x);
	float dy = fabsf(child->y - layout->y);
	float c = sqrtf(dx * dx + dy * dy);

	float sn = dx / c;
	float cs = dy / c;

	ImVec2_c prevPos = (ImVec2_c){
		layout->x, layout->y
	};

	prevPos.x += NODE_RAD * sn * (prevPos.x > child->x ? -1 : 1);
	prevPos.y += NODE_RAD * cs;

	CTX_AddCmd(ctx, (DrawCmd) { CMD_LINE, prevPos.x, prevPos.y, child->x, child->y, 0xFFCFCFFF, 3 });
}

void R_EmitWidgets(Node* root, LayoutNode* layout) {
	if (!root || !layout) return;

	for (int i = 0; i < root->childCount; i++) {
		R_AddConnectingLine(layout, i);
		R_EmitWidgets(root->children[i], layout->children[i]);
	}

	R_TreeWidget(root, layout->x, layout->y);
}

void R_BackendRender() {
	for (int i = 0; i < ctx->drawCmdCount; i++) {
		CMD_ProcessCommand(&ctx->drawCmds[i]);
	}
}

void R_TreeFrame(Node* root, float x, float y) {
	ImGuiIO* io = igGetIO();
	ctx->mouseX = io->MousePos.x;
	ctx->mouseY = io->MousePos.y;
	ctx->mouseDown = io->MouseDown[0];
	ctx->mousePressed = io->MouseClicked[0];
	ctx->mouseReleased = io->MouseReleased[0];

	A_Reset(ctx->frameArena);

	ctx->drawCmdCount = 0;
	ctx->hotId = 0;

	float cursorX = x;
	LayoutNode* node = A_Alloc(ctx->frameArena, sizeof(LayoutNode));
	L_LayoutTree(root, y, &cursorX, ctx, node);

	R_EmitWidgets(root, node);

	R_BackendRender();
}