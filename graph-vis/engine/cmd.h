#ifndef GRAPH_VIS_CMD
#define GRAPH_VIS_CMD

#include <stdint.h>

typedef enum { CMD_LINE, CMD_RECT, CMD_TEXT, CMD_NUM, CMD_CIRCLE } CmdType;

typedef struct DrawCmd {
	CmdType type;
	float x0, y0, x1, y1;
	uint32_t color;
	union {
		const char* text;
		int num;
	};
} DrawCmd;

void CMD_ProcessCommand(DrawCmd* cmd);

#endif