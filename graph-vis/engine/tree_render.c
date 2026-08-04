#include "window.h"
#include "tree_render.h"
#include "ds/list.h"
#include "ds/node.h"
#include "ds/bt.h"
#include "context.h"
#include "layout.h"

#include <assert.h>
#include <math.h>

#define NODE_RAD 25
#define NODE_DIM 50

#define ARENA_SIZE 64 * 1024
#define DRAW_CMDS_SIZE 256
#define TREE_CMDS_SIZE 128
#define VALUE_SIZE 6

static ImDrawList *dl;
static LayoutNode *rootLayout;
static UIContext *ctx;
static char *valueBuffer;

extern Node *sentinel;
extern TreeType treeType;
extern BT *tree;

void R_Init() {
    Arena *arena = malloc(sizeof(Arena));
    assert(arena != NULL);

    *arena = (Arena){malloc(ARENA_SIZE), ARENA_SIZE, 0};

    UIStateMap *map = malloc(sizeof(UIStateMap));
    assert(map != NULL);

    *map = (UIStateMap){calloc(64, sizeof(NodeUIState)), 64};

    ctx = malloc(sizeof(UIContext));
    assert(ctx != NULL);

    *ctx = (UIContext){
        .frameArena = arena,
        .uiStateMap = map,
        .drawCmds = malloc(DRAW_CMDS_SIZE * sizeof(DrawCmd)),
        .drawCmdCap = DRAW_CMDS_SIZE,
        .treeCmds = malloc(TREE_CMDS_SIZE * sizeof(TreeCmd)),
        .treeCmdCap = TREE_CMDS_SIZE,
    };

    valueBuffer = calloc(VALUE_SIZE, sizeof(char));
}

int R_TreeWidget(const Node *node, float x, float y) {
    if (!node || node == sentinel) return 0;

    NodeUIState *st = CTX_UIGetOrCreate(ctx->uiStateMap, node->id);

    float w = NODE_RAD, h = NODE_RAD;
    int hovered = !ctx->mouseCaptured &&
                  ctx->mouseX >= x - w && ctx->mouseX <= x + w &&
                  ctx->mouseY >= y - h && ctx->mouseY <= y + h;

    if (hovered) ctx->hotId = node->id;
    int clicked = 0;
    if (hovered && ctx->mousePressed) ctx->activeId = node->id;
    if (ctx->activeId == node->id && ctx->mouseReleased) {
        clicked = hovered; // released while still over it
        ctx->activeId = 0;
    }

    int isPopupTarget = ctx->popupNode == node;
    uint32_t color = (ctx->activeId == node->id || isPopupTarget)
                         ? 0xFFFF0000
                         : (ctx->hotId == node->id)
                               ? 0xFFFFFF00
                               : node->color;

    CTX_AddCmd(ctx, (DrawCmd){CMD_CIRCLE, x, y, w, 0, color, NULL});
    CTX_AddCmd(ctx, (DrawCmd){CMD_NUM, x, y, 0, 0, FONT_COLOR, .num = node->val});

    if (treeType == AVL) {
        // display balance factor
        CTX_AddCmd(ctx, (DrawCmd){
                       CMD_NUM, x + NODE_RAD * 0.75f, y - NODE_RAD * 0.75f, 0, 0, FONT_COLOR, .num = node->balance
                   });
        CTX_AddCmd(ctx, (DrawCmd){
                       CMD_NUM, x - NODE_RAD * 0.75f, y - NODE_RAD * 0.75f, 0, 0, FONT_COLOR, .num = node->height
                   });
    }

    if (clicked) {
        st->expanded = !st->expanded;
        ctx->popupNode = (Node *) node;
        ctx->popupRequestOpen = 1;
    }
    st->x = x;
    st->y = y;
    st->id = node->id;

    return clicked;
}

void R_AddConnectingLine(LayoutNode *layout, int childIdx) {
    LayoutNode *child = layout->children[childIdx];
    if (!child) return;

    float dx = fabsf(child->x - layout->x);
    float dy = fabsf(child->y - layout->y);
    float c = sqrtf(dx * dx + dy * dy);

    float sn = dx / c;
    float cs = dy / c;

    ImVec2_c prevPos = (ImVec2_c){
        .x = layout->x, .y = layout->y
    };

    prevPos.x += NODE_RAD * sn * (prevPos.x > child->x ? -1.f : 1.f);
    prevPos.y += NODE_RAD * cs;

    CTX_AddCmd(ctx, (DrawCmd){CMD_LINE, prevPos.x, prevPos.y, child->x, child->y, FONT_COLOR, .num = 3});
}

void R_EmitWidgets(Node *root, LayoutNode *layout) {
    if (!root || root == sentinel || !layout) return;

    for (int i = 0; i < root->childCount; i++) {
        Node *child = root->children[i];
        if (!child || child == sentinel) continue;

        R_AddConnectingLine(layout, i);
        R_EmitWidgets(child, layout->children[i]);
    }

    R_TreeWidget(root, layout->x, layout->y);
}

void R_BackendRender() {
    for (int i = 0; i < ctx->treeCmdCount; i++) {
        CMD_ProcessTreeCommand(&ctx->treeCmds[i]);
    }

    for (int i = 0; i < ctx->drawCmdCount; i++) {
        CMD_ProcessDrawCommand(&ctx->drawCmds[i]);
    }
}

static int StringToInt(char buffer[VALUE_SIZE]) {
    int i = 0;
    int isNegative = buffer[0] == '-' ? -1 : 1;
    if (isNegative == -1) {
        i++;
    }
    int res = buffer[i++] - 48;

    while (buffer[i] != '\0') {
        res = res * 10 + (buffer[i++] - 48);
    }

    return isNegative * res;
}

void R_EmitNodePopup(BT *bt) {
    if (ctx->popupRequestOpen) {
        igOpenPopup_Str("Node Info", 0);
        ctx->popupRequestOpen = 0;
    }

    if (!igBeginPopup("Node Info", 0)) return;

    Node *n = ctx->popupNode;
    if (n) {
        igText("Value: %d", n->val);
        if (treeType == RED_BLACK) igText("Color: %s", n->color == RED ? "Red" : "Black");

        Node *par = (n->par && n->par != sentinel) ? n->par : NULL;
        if (!par) {
            igText("Siblings: none (root)");
        } else {
            for (size_t i = 0; i < par->childCount; i++) {
                Node *sib = par->children[i];
                if (!sib || sib == sentinel || sib == n) continue;
                igText("Sibling: %d", sib->val);
            }
        }

        igSeparator();
        if (igButton("Delete", (ImVec2_c){0, 0})) {
            CTX_AddTreeCmd(ctx, (TreeCmd){
                               .cmdType = CMD_TREE_DEL,
                               .treeType = treeType,
                               .tree = bt,
                               .node = n,
                           });
            ctx->popupNode = NULL;
            igCloseCurrentPopup();
        }
    }

    igEndPopup();
}

void R_EmitControlWidgets(BT *bt) {
    // render control widgets for the trees/graphs
    igBegin("Control Window", NULL, 0);

    igInputText("Value", valueBuffer, VALUE_SIZE, 0, 0, 0);

    if (igButton("Add", (ImVec2_c){0, 0})) {
        CTX_AddTreeCmd(ctx, (TreeCmd){
                           .cmdType = CMD_TREE_ADD,
                           .treeType = treeType,
                           .val = StringToInt(valueBuffer),
                           .tree = bt,
                       });
    }

    igEnd();
}

void R_TreeFrame(BT *bt, float x, float y) {
    ImGuiIO *io = igGetIO();
    ctx->mouseX = io->MousePos.x;
    ctx->mouseY = io->MousePos.y;
    ctx->mouseDown = io->MouseDown[0];
    ctx->mousePressed = io->MouseClicked[0];
    ctx->mouseReleased = io->MouseReleased[0];
    ctx->mouseCaptured = io->WantCaptureMouse;

    A_Reset(ctx->frameArena);

    ctx->drawCmdCount = 0;
    ctx->treeCmdCount = 0;
    ctx->hotId = 0;

    float cursorX = x;
    LayoutNode *node = A_Alloc(ctx->frameArena, sizeof(LayoutNode));
    L_LayoutTree(bt->root, y, &cursorX, ctx, node);

    R_EmitWidgets(bt->root, node);
    R_EmitControlWidgets(bt);
    R_EmitNodePopup(bt);

    R_BackendRender();
}
