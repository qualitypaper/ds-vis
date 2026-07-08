#ifndef GRAPH_VIS_CONTEXT
#define GRAPH_VIS_CONTEXT

#include "defs.h"
#include "arena.h"
#include "cmd.h"
#include <stdbool.h>

typedef struct NodeUIState {
	ID id;
	bool expanded;
	float x, y;
} NodeUIState;

typedef struct UIStateMap {
	NodeUIState* slots;
	int capacity;
} UIStateMap;

typedef struct UIContext {
	Arena* frameArena;
	UIStateMap* uiStateMap;

	float mouseX, mouseY;
	bool mouseDown, mousePressed, mouseReleased;

	ID hotId;
	ID activeId;

	DrawCmd* drawCmds;
	int drawCmdCount;
	int drawCmdCap;
} UIContext;

void CTX_AddCmd(UIContext* ctx, DrawCmd cmd);
NodeUIState* CTX_UIGetOrCreate(UIStateMap* uiState, ID elemId);

#endif