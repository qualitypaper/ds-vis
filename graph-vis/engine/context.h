#ifndef GRAPH_VIS_CONTEXT
#define GRAPH_VIS_CONTEXT

#include "defs.h"
#include "arena.h"
#include "cmd.h"

typedef struct NodeUIState {
	ID id;
	int8_t expanded;
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
	int8_t mouseDown, mousePressed, mouseReleased;
	int8_t mouseCaptured; /* ImGui window/popup wants the mouse; suppress manual node hit-testing */

	ID hotId;
	ID activeId;

	struct Node* popupNode;
	int8_t popupRequestOpen;

	DrawCmd* drawCmds;
	int drawCmdCount;
	int drawCmdCap;

	TreeCmd* treeCmds;
	int treeCmdCount;
	int treeCmdCap;

} UIContext;

void CTX_AddCmd(UIContext* ctx, DrawCmd cmd);
void CTX_AddTreeCmd(UIContext* ctx, TreeCmd cmd);
NodeUIState* CTX_UIGetOrCreate(UIStateMap* uiState, ID elemId);

#endif