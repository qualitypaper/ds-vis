#include "cmd.h"
#include "window.h"

#define FONT_SIZE 30

void CMD_ProcessCommand(DrawCmd* cmd)
{
	ImDrawList* dl = igGetForegroundDrawList_ViewportPtr(NULL);
	ImVec2_c p0 = { cmd->x0, cmd->y0 };
	ImVec2_c p1 = { cmd->x1, cmd->y1 };

	switch (cmd->type) {
	case CMD_LINE:
		ImDrawList_AddLine(dl, p0, p1, cmd->color, cmd->num);
		break;
	case CMD_RECT:
		ImDrawList_AddRect(dl, p0, p1, cmd->color, 0.0f, 1.0f, 0);
		break;
	case CMD_TEXT:
		if (cmd->text)
			ImDrawList_AddText_Vec2(dl, p0, cmd->color, cmd->text, NULL);
		break;
	case CMD_CIRCLE:
		ImDrawList_AddCircleFilled(dl, p0, cmd->x1, cmd->color, 0);
		break;
	case CMD_NUM: {
		char buf[6];
		snprintf(buf, sizeof(buf), "%d", cmd->num);

		ImVec2_c size = igCalcTextSize(buf, NULL, false, 0);

		ImVec2_c endPos = { p0.x - size.x, p0.y - size.y };

		ImDrawList_AddText_FontPtr(dl, igGetFont(), FONT_SIZE, endPos, cmd->color, buf, NULL, 0.0f, NULL);
	} break;
	default: break;
	}
}
