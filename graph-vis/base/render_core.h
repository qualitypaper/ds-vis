//
// Created by qualitypaper on 8/6/26.
//

#ifndef GRAPH_VIS_RENDER_CORE_H
#define GRAPH_VIS_RENDER_CORE_H

#include "base/core.h"

typedef enum RenderCellType RenderCellType;

enum RenderCellType { CIRCLE, SQUARE, FILLED_CIRCLE, FILLED_SQUARE, RECT, FILLED_RECT, TRIAN, FILLED_TRIAN, TEXT, SNUM, UNUM };


typedef struct RenderCell RenderCell;

struct RenderCell
{
    void* v;
    U64 valueSize;
    F32 x, y;
    union
    {
        U32 radius;
        U32 width;
        U32 fontSize;
    };
    U32 height;
    U32 rotation;
    U32 color;
    U32 borderColor;
    RenderCellType type;
};



#endif //GRAPH_VIS_RENDER_CORE_H
