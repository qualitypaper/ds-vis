#include "context.h"

#define DRAW_CMDS_SIZE 256
#define TREE_CMDS_SIZE 128
#define UI_SLOTS 64

UIContext* context_init(Arena* arena)
{
	UIContext* ctx = PushStruct(arena, UIContext);

	UIStateMap* map = PushStruct(arena, UIStateMap);
	map->slots = PushArray(arena, NodeUIState, UI_SLOTS);
	map->capacity = UI_SLOTS;

	ctx->frameArena = arena;
	ctx->uiStateMap = map;
	ctx->drawCmds = PushArray(arena, DrawCmd, DRAW_CMDS_SIZE);
	ctx->drawCmdCap = DRAW_CMDS_SIZE;
	ctx->treeCmds = PushArray(arena, TreeCmd, TREE_CMDS_SIZE);
	ctx->treeCmdCap = TREE_CMDS_SIZE;

	return ctx;
}

void CTX_AddCmd(UIContext* ctx, DrawCmd cmd)
{
	ctx->drawCmds[ctx->drawCmdCount++] = cmd;
}

void CTX_AddTreeCmd(UIContext* ctx, TreeCmd cmd) {
	ctx->treeCmds[ctx->treeCmdCount++] = cmd;
}

NodeUIState* CTX_UIGetOrCreate(UIStateMap* uiState, ID elemId)
{
	int start = (int)((elemId >> 4) % (ID)uiState->capacity);
	for (int i = 0; i < uiState->capacity; i++) {
		int idx = (start + i) % uiState->capacity;
		if (uiState->slots[idx].id == elemId) return &uiState->slots[idx];
		if (uiState->slots[idx].id == 0) {
			uiState->slots[idx].id = elemId;
			return &uiState->slots[idx];
		}
	}
	return NULL; // ponytail: map full; bump capacity if tree grows past 64 nodes
}
