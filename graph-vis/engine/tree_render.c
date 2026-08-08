#include "window.h"
#include "tree_render.h"
#include "dsa/node.h"
#include "dsa/bt.h"
#include "context.h"
#include "base/core.h"

#include <assert.h>
#include <math.h>

#define NODE_RAD 25
#define NODE_DIM 50

#define ARENA_SIZE KB(64)
#define VALUE_SIZE 6

static UIContext* ctx;
static char* valueBuffer;

void render_init()
{
    Arena* persistentArena = arena_alloc(ARENA_SIZE); /* UIContext, uiStateMap, command buffers */
    Arena* frameArena = arena_alloc(ARENA_SIZE); /* per-frame LayoutNodes */

    ctx = context_init(persistentArena);
    ctx->frameArena = frameArena;

    valueBuffer = PushArrayNoZero(persistentArena, char, VALUE_SIZE);
    MemorySet(valueBuffer, 0, VALUE_SIZE);
}

S32 tree_widget(const BNode* node, F32 x, F32 y, BT* bt)
{
    if (!IsValid(node)) return 0;

    NodeUIState* st = CTX_UIGetOrCreate(ctx->uiStateMap, node->id);

    F32 w = NODE_RAD, h = NODE_RAD;
    S8 hovered = !ctx->mouseCaptured &&
        ctx->mouseX >= x - w && ctx->mouseX <= x + w &&
        ctx->mouseY >= y - h && ctx->mouseY <= y + h;

    if (hovered) ctx->hotId = node->id;
    S8 clicked = 0;
    if (hovered && ctx->mousePressed) ctx->activeId = node->id;
    if (ctx->activeId == node->id && ctx->mouseReleased)
    {
        clicked = hovered; // released while still over it
        ctx->activeId = 0;
    }

    S8 isPopupTarget = ctx->popupNode == node;
    COLOR color = (ctx->activeId == node->id || isPopupTarget)
                      ? 0xFFFF0000
                      : (ctx->hotId == node->id)
                      ? 0xFFFFFF00
                      : node->color;

    CTX_AddCmd(ctx, (DrawCmd){CMD_CIRCLE, x, y, w, 0, color, NULL});
    CTX_AddCmd(ctx, (DrawCmd){CMD_NUM, x, y, 0, 0, FONT_COLOR, .num = node->val});

    if (bt->type == AVL)
    {
        // display balance factor
        CTX_AddCmd(ctx, (DrawCmd){
                       CMD_NUM, x + NODE_RAD * 0.75f, y - NODE_RAD * 0.75f, 0, 0, FONT_COLOR, .num = node->balance
                   });
        CTX_AddCmd(ctx, (DrawCmd){
                       CMD_NUM, x - NODE_RAD * 0.75f, y - NODE_RAD * 0.75f, 0, 0, FONT_COLOR, .num = node->height
                   });
    }

    if (clicked)
    {
        st->expanded = !st->expanded;
        ctx->popupNode = (BNode*)node;
        ctx->popupRequestOpen = 1;
    }
    st->x = x;
    st->y = y;
    st->id = node->id;

    return clicked;
}

void add_connecting_line(LayoutTreeNode* layout, int childIdx)
{
    LayoutTreeNode* child = layout->children[childIdx];
    if (!child) return;

    F32 dx = fabsf(child->x - layout->x);
    F32 dy = fabsf(child->y - layout->y);
    F32 c = sqrtf(dx * dx + dy * dy);

    F32 sn = dx / c;
    F32 cs = dy / c;

    ImVec2_c prevPos = (ImVec2_c){
        .x = layout->x, .y = layout->y
    };

    prevPos.x += NODE_RAD * sn * (prevPos.x > child->x ? -1.f : 1.f);
    prevPos.y += NODE_RAD * cs;

    CTX_AddCmd(ctx, (DrawCmd){CMD_LINE, prevPos.x, prevPos.y, child->x, child->y, FONT_COLOR, .num = 3});
}

void emit_widgets(BNode* root, LayoutTreeNode* layout, BT* bt)
{
    if (!IsValid(root) || !layout) return;

    if (IsValid(root->left))
    {
        add_connecting_line(layout, 0);
        emit_widgets(root->left, layout->children[0], bt);
    }
    if (IsValid(root->right))
    {
        add_connecting_line(layout, 1);
        emit_widgets(root->right, layout->children[1], bt);
    }

    tree_widget(root, layout->x, layout->y, bt);
}

void backend_render()
{
    for (int i = 0; i < ctx->treeCmdCount; i++)
    {
        CMD_ProcessTreeCommand(&ctx->treeCmds[i]);
    }

    for (int i = 0; i < ctx->drawCmdCount; i++)
    {
        CMD_ProcessDrawCommand(&ctx->drawCmds[i]);
    }
}

static int StringToInt(char buffer[VALUE_SIZE])
{
    int i = 0;
    int isNegative = buffer[0] == '-' ? -1 : 1;
    if (isNegative == -1)
    {
        i++;
    }
    int res = buffer[i++] - 48;

    while (buffer[i] != '\0')
    {
        res = res * 10 + (buffer[i++] - 48);
    }

    return isNegative * res;
}

void emit_node_popup(BT* bt)
{
    if (ctx->popupRequestOpen)
    {
        igOpenPopup_Str("Node Info", 0);
        ctx->popupRequestOpen = 0;
    }

    if (!igBeginPopup("Node Info", 0)) return;

    BNode* n = ctx->popupNode;
    if (n)
    {
        igText("Value: %d", n->val);
        if (bt->type == RED_BLACK) igText("Color: %s", n->color == RED ? "Red" : "Black");

        const BNode* par = n->par;

        if (!IsValid(par))
        {
            igText("Siblings: none (root)");
        }
        else
        {
            const BNode* sib = BIsLeftChild(n) ? par->right : par->left;
            if (IsValid(sib) && sib != n)
                igText("Sibling: %d", sib->val);
            else
                igText("No siblings");
        }

        igSeparator();
        if (igButton("Delete", (ImVec2_c){0, 0}))
        {
            CTX_AddTreeCmd(ctx, (TreeCmd){
                               .cmdType = CMD_TREE_DEL,
                               .tree = bt,
                               .node = n,
                           });
            ctx->popupNode = NULL;
            igCloseCurrentPopup();
        }
    }

    igEndPopup();
}

void emit_control_widgets(BT* bt)
{
    // render control widgets for the trees/graphs
    igBegin("Control Window", NULL, 0);

    igInputText("Value", valueBuffer, VALUE_SIZE, 0, 0, 0);

    if (igButton("Add", (ImVec2_c){0, 0}))
    {
        CTX_AddTreeCmd(ctx, (TreeCmd){
                           .cmdType = CMD_TREE_ADD,
                           .val = StringToInt(valueBuffer),
                           .tree = bt,
                       });
    }

    igEnd();
}

void emit_tree_type_popup(BT* bt)
{
    igBegin("Tree Type Popup", NULL, 0);

    switch (bt->type)
    {
    case BST:
        {
            if (igButton("To Red-Black", (ImVec2_c){0, 0}))
            {
                CTX_AddTreeCmd(ctx, (TreeCmd){
                                   .cmdType = CMD_TREE_TRANSFORM,
                                   .tree = bt,
                                   .transform = BST_2_RB,
                               });
            }
            if (igButton("To AVL", (ImVec2_c){0, 0}))
            {
                CTX_AddTreeCmd(ctx, (TreeCmd){
                                   .cmdType = CMD_TREE_TRANSFORM,
                                   .tree = bt,
                                   .transform = BST_2_AVL,
                               });
            }
        }
        break;
    case RED_BLACK:
        {
            if (igButton("To BST", (ImVec2_c){0, 0}))
            {
                CTX_AddTreeCmd(ctx, (TreeCmd){
                                   .cmdType = CMD_TREE_TRANSFORM,
                                   .tree = bt,
                                   .transform = RB_2_BST,
                               });
            }
            if (igButton("To AVL", (ImVec2_c){0, 0}))
            {
                CTX_AddTreeCmd(ctx, (TreeCmd){
                                   .cmdType = CMD_TREE_TRANSFORM,
                                   .tree = bt,
                                   .transform = RB_2_AVL,
                               });
            }
        }
        break;
    case AVL:
        {
            if (igButton("To BST", (ImVec2_c){0, 0}))
            {
                CTX_AddTreeCmd(ctx, (TreeCmd){
                                   .cmdType = CMD_TREE_TRANSFORM,
                                   .tree = bt,
                                   .transform = AVL_2_BST,
                               });
            }
            if (igButton("To Red-Black", (ImVec2_c){0, 0}))
            {
                CTX_AddTreeCmd(ctx, (TreeCmd){
                                   .cmdType = CMD_TREE_TRANSFORM,
                                   .tree = bt,
                                   .transform = AVL_2_RB,
                               });
            }
        }
        break;
    default: break;
    }

    igEnd();
}

void tree_frame(BT* bt, F32 x, F32 y)
{
    ImGuiIO* io = igGetIO();
    ctx->mouseX = io->MousePos.x;
    ctx->mouseY = io->MousePos.y;
    ctx->mouseDown = io->MouseDown[0];
    ctx->mousePressed = io->MouseClicked[0];
    ctx->mouseReleased = io->MouseReleased[0];
    ctx->mouseCaptured = io->WantCaptureMouse;

    arena_reset(ctx->frameArena);

    ctx->drawCmdCount = 0;
    ctx->treeCmdCount = 0;
    ctx->hotId = 0;

    LayoutTreeNode* node = PushStruct(ctx->frameArena, LayoutTreeNode);
    layout_tree(bt->root, y, x, ctx, node);

    emit_widgets(bt->root, node, bt);
    emit_control_widgets(bt);
    emit_node_popup(bt);
    emit_tree_type_popup(bt);

    backend_render();
}
