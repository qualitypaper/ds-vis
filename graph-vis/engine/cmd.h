#ifndef GRAPH_VIS_CMD
#define GRAPH_VIS_CMD

#include "base/core.h"
#include "dsa/bt.h"

typedef enum { CMD_LINE, CMD_RECT, CMD_TEXT, CMD_NUM, CMD_CIRCLE } DrawCmdType;
typedef enum { CMD_TREE_ADD, CMD_TREE_DEL, CMD_TREE_TRANSFORM} TreeCmdType;
typedef enum { BST_2_RB, BST_2_AVL, RB_2_BST, RB_2_AVL, AVL_2_BST, AVL_2_RB} TreeTransformationType;

typedef struct {
	DrawCmdType type;
	F32 x0, y0, x1, y1;
	U32 color;
	union {
		const char* text;
		S32 num;
	};
} DrawCmd;

typedef struct {
	TreeCmdType cmdType;
	BT* tree;
	union {
		S32 val;
		BNode* node;
		TreeTransformationType transform;
	};
} TreeCmd;

void CMD_ProcessDrawCommand(const DrawCmd* cmd);
void CMD_ProcessTreeCommand(const TreeCmd* cmd);

#endif