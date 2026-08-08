#include "layout.h"

#define NODE_SPACING  60.0f
#define LEVEL_SPACING 60.0f

static F32 layout_tree_core(BNode* node, F32 depthY, F32* cursorX, UIContext* ctx, LayoutTreeNode* out)
{
    out->node = node;
    out->y = depthY;
    out->children[0] = NULL;
    out->children[1] = NULL;

    int hasLeft = IsValid(node->left);
    int hasRight = IsValid(node->right);
    if (!hasLeft && hasRight) *cursorX += NODE_SPACING; /* phantom left slot */

    F32 first = 0, last = 0;
    int placed = 0;

    if (IsValid(node->left))
    {
        LayoutTreeNode* child = PushStruct(ctx->frameArena, LayoutTreeNode);
        out->children[0] = child;
        F32 cx = layout_tree_core(node->left, depthY + LEVEL_SPACING, cursorX, ctx, child);
        if (placed == 0) first = cx;
        last = cx;
        placed++;
    }
    if (IsValid(node->right))
    {
        LayoutTreeNode* child = PushStruct(ctx->frameArena, LayoutTreeNode);
        out->children[1] = child;
        F32 cx = layout_tree_core(node->right, depthY + LEVEL_SPACING, cursorX, ctx, child);
        if (placed == 0) first = cx;
        last = cx;
        placed++;
    }

    if (placed == 0)
    {
        out->x = *cursorX;
        *cursorX += NODE_SPACING;
    }
    else if (placed == 1)
    {
        if (out->children[0])
        {
            /* only left child: parent must sit right of it; reserve phantom right slot */
            out->x = first + NODE_SPACING * 0.5f;
            *cursorX += NODE_SPACING;
        }
        else
        {
            /* only right child: phantom left already reserved above */
            out->x = last - NODE_SPACING * 0.5f;
        }
    }
    else
    {
        out->x = (first + last) * 0.5f;
    }
    out->subtreeWidth = last - first;
    return out->x;
}

static void shift_x(LayoutTreeNode* n, F32 dx)
{
    if (!n) return;
    n->x += dx;
    shift_x(n->children[0], dx);
    shift_x(n->children[1], dx);
}

void layout_tree(BNode* node, F32 depthY, F32 cursorX, UIContext* ctx, LayoutTreeNode* out)
{
    /* pack from 0, then translate so the root (not the leftmost child) lands on *cursorX */
    F32 packX = 0.0f;
    F32 rootX = layout_tree_core(node, depthY, &packX, ctx, out);
    shift_x(out, cursorX - rootX);
}
