#ifndef DRAW_CORE_GRAPH_VIS_H
#define DRAW_CORE_GRAPH_VIS_H

#include <cimgui.h>
#include <stdio.h>

#define DrawLine(x0, y0, x1, y1, col, thick) do { \
    ImVec2_c p0_ = { .x = (x0), .y = (y0) }; \
    ImVec2_c p1_ = { .x = (x1), .y = (y1) }; \
    ImDrawList_AddLine(igGetBackgroundDrawList(NULL), p0_, p1_, (col), (thick)); \
} while (0)

#define DrawRect(x0, y0, x1, y1, col) do { \
    ImVec2_c p0_ = { .x = (x0), .y = (y0) }; \
    ImVec2_c p1_ = { .x = (x1), .y = (y1) }; \
    ImDrawList_AddRect(igGetBackgroundDrawList(NULL), p0_, p1_, (col), 0.0f, 1.0f, 0); \
} while (0)

#define DrawCircle(cx, cy, radius, col) do { \
    ImVec2_c c_ = { .x = (cx), .y = (cy) }; \
    ImDrawList_AddCircleFilled(igGetBackgroundDrawList(NULL), c_, (radius), (col), 0); \
} while (0)

#define DrawText(tx, ty, col, text) do { \
    ImVec2_c pos_ = { .x = (tx), .y = (ty) }; \
    ImDrawList_AddText_Vec2(igGetBackgroundDrawList(NULL), pos_, (col), (text), NULL); \
} while (0)

#define DrawNumber(nx, ny, col, num, font_size) do { \
    char buf_[6]; \
    snprintf(buf_, sizeof(buf_), "%d", (num)); \
    ImVec2_c pos_ = { .x = (nx), .y = (ny) }; \
    ImVec2_c size_ = igCalcTextSize(buf_, NULL, false, 0); \
    ImVec2_c endPos_ = { pos_.x - size_.x, pos_.y - size_.y }; \
    ImDrawList_AddText_FontPtr(igGetBackgroundDrawList(NULL), igGetFont(), (font_size), endPos_, (col), buf_, NULL, 0.0f, NULL); \
} while (0)
#endif