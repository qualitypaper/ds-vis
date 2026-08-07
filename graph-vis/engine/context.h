#ifndef GRAPH_VIS_CONTEXT
#define GRAPH_VIS_CONTEXT

#include "base/arena.h"
#include "base/core.h"
#include "cmd.h"

typedef struct NodeUIState {
	ID id;
	S8 expanded;
	F32 x, y;
} NodeUIState;

typedef struct UIStateMap {
	NodeUIState* slots;
	S32 capacity;
} UIStateMap;

typedef struct UIContext {
	Arena* frameArena;
	UIStateMap* uiStateMap;

	F32 mouseX, mouseY;
	S8 mouseDown, mousePressed, mouseReleased;
	S8 mouseCaptured; /* ImGui window/popup wants the mouse; suppress manual node hit-testing */

	ID hotId;
	ID activeId;

	Node* popupNode;
	S8 popupRequestOpen;

	DrawCmd* drawCmds;
	S32 drawCmdCount;
	S32 drawCmdCap;

	TreeCmd* treeCmds;
	S32 treeCmdCount;
	S32 treeCmdCap;

} UIContext;

UIContext* context_init(Arena* arena);

void CTX_AddCmd(UIContext* ctx, DrawCmd cmd);
void CTX_AddTreeCmd(UIContext* ctx, TreeCmd cmd);
NodeUIState* CTX_UIGetOrCreate(UIStateMap* uiState, ID elemId);

#endif
