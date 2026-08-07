#include "cmd.h"
#include "window.h"
#include "dsa/bt.h"
#include "base/draw_core.h"

#define FONT_SIZE 30

void CMD_ProcessDrawCommand(const DrawCmd* cmd)
{
    switch (cmd->type)
    {
    case CMD_LINE:
        DrawLine(cmd->x0, cmd->y0, cmd->x1, cmd->y1, cmd->color, cmd->num);
        break;
    case CMD_RECT:
        DrawRect(cmd->x0, cmd->y0, cmd->x1, cmd->y1, cmd->color);
        break;
    case CMD_TEXT:
        if (cmd->text)
            DrawText(cmd->x0, cmd->y0, cmd->color, cmd->text);
        break;
    case CMD_CIRCLE:
        DrawCircle(cmd->x0, cmd->y0, cmd->x1, cmd->color);
        break;
    case CMD_NUM:
        DrawNumber(cmd->x0, cmd->y0, cmd->color, cmd->num, FONT_SIZE);
        break;
    default: break;
    }
}

void CMD_ProcessTreeCommand(const TreeCmd* cmd)
{
    switch (cmd->tree->type)
    {
    case BINARY:
        switch (cmd->cmdType)
        {
        case CMD_TREE_ADD:
            BT_Add(cmd->val, cmd->tree);
            break;
        case CMD_TREE_DEL:
            BT_Del(cmd->node, cmd->tree);
            break;
        default: break;
        }
        break;

    case BST:
        switch (cmd->cmdType)
        {
        case CMD_TREE_ADD:
            BST_Add(cmd->val, cmd->tree);
            break;
        case CMD_TREE_DEL:
            BST_Del(cmd->node, cmd->tree);
            break;
        case CMD_TREE_TRANSFORM:
            switch (cmd->transform)
            {
            case BST_2_RB:  BST2RB(cmd->tree);  break;
            case BST_2_AVL: BST2AVL(cmd->tree); break;
            default: break;
            }
            break;
        }
        break;

    case RED_BLACK:
        switch (cmd->cmdType)
        {
        case CMD_TREE_ADD:
            RB_Add(cmd->val, cmd->tree);
            break;
        case CMD_TREE_DEL:
            RB_Del(cmd->node, cmd->tree);
            break;
        case CMD_TREE_TRANSFORM:
            switch (cmd->transform)
            {
            case RB_2_BST: RB2BST(cmd->tree); break;
            case RB_2_AVL: RB2AVL(cmd->tree); break;
            default: break;
            }
            break;
        }
        break;

    case AVL:
        switch (cmd->cmdType)
        {
        case CMD_TREE_ADD:
            AVL_Add(cmd->val, cmd->tree);
            break;
        case CMD_TREE_DEL:
            AVL_Del(cmd->node, cmd->tree);
            break;
        case CMD_TREE_TRANSFORM:
            switch (cmd->transform)
            {
            case AVL_2_BST: AVL2BST(cmd->tree); break;
            case AVL_2_RB:  AVL2RB(cmd->tree);  break;
            default: break;
            }
            break;
        }
        break;
    }
}
