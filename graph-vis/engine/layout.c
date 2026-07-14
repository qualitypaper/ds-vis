#include "layout.h"

#define NODE_SPACING  60.0f
#define LEVEL_SPACING 60.0f

extern Node* sentinel; /* RB leaves point here, not NULL */

/* A real child is non-NULL and not the shared RB sentinel (which self-references). */
static int is_child(const Node* c) { return c && c != sentinel; }

static float layout_core(Node* node, float depthY, float* cursorX, UIContext* ctx, LayoutNode* out)
{
	out->node = node;
	out->y = depthY;
	out->children[0] = NULL;
	out->children[1] = NULL;

	int hasLeft = node->childCount > 0 && is_child(node->children[0]);
	int hasRight = node->childCount > 1 && is_child(node->children[1]);
	if (!hasLeft && hasRight) *cursorX += NODE_SPACING; /* phantom left slot */

	float first = 0, last = 0;
	int   placed = 0;

	for (size_t i = 0; i < node->childCount; i++) {
		if (!is_child(node->children[i])) continue;
		LayoutNode* child = A_Alloc(ctx->frameArena, sizeof(LayoutNode));
		out->children[i] = child;
		float cx = layout_core(node->children[i], depthY + LEVEL_SPACING, cursorX, ctx, child);
		if (placed == 0) first = cx;
		last = cx;
		placed++;
	}

	if (placed == 0) {
		out->x = *cursorX;
		*cursorX += NODE_SPACING;
	}
	else if (placed == 1) {
		if (out->children[0]) {
			/* only left child: parent must sit right of it; reserve phantom right slot */
			out->x = first + NODE_SPACING * 0.5f;
			*cursorX += NODE_SPACING;
		}
		else {
			/* only right child: phantom left already reserved above */
			out->x = last - NODE_SPACING * 0.5f;
		}
	}
	else {
		out->x = (first + last) * 0.5f;
	}
	out->subtreeWidth = last - first;
	return out->x;
}

static void shift_x(LayoutNode* n, float dx)
{
	if (!n) return;
	n->x += dx;
	shift_x(n->children[0], dx);
	shift_x(n->children[1], dx);
}

float L_LayoutTree(Node* node, float depthY, float* cursorX, UIContext* ctx, LayoutNode* out)
{
	/* pack from 0, then translate so the root (not the leftmost child) lands on *cursorX */
	float desiredX = *cursorX;
	float packX = 0.0f;
	float rootX = layout_core(node, depthY, &packX, ctx, out);
	shift_x(out, desiredX - rootX);
	return desiredX;
}
