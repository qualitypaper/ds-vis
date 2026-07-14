#ifndef GRAPH_VIS_CMD
#define GRAPH_VIS_CMD

#include <stdint.h>

#include "ds/bt.h"

typedef enum { CMD_LINE, CMD_RECT, CMD_TEXT, CMD_NUM, CMD_CIRCLE } DrawCmdType;
typedef enum { CMD_TREE_ADD, CMD_TREE_DEL } TreeCmdType;

typedef struct {
	DrawCmdType type;
	float x0, y0, x1, y1;
	uint32_t color;
	union {
		const char* text;
		int num;
	};
} DrawCmd;

typedef struct {
	TreeCmdType cmdType;
	TreeType treeType;
	BT* tree;
	union {
		int val;
		Node* node;
	};
} TreeCmd;

void CMD_ProcessDrawCommand(DrawCmd* cmd);
void CMD_ProcessTreeCommand(TreeCmd* cmd);

#endif