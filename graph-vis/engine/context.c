#include "context.h"

void CTX_AddCmd(UIContext* ctx, DrawCmd cmd)
{
	ctx->drawCmds[ctx->drawCmdCount++] = cmd;
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
