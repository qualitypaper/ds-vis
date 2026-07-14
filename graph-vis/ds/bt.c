#include "bt.h"
#include "queue.h"
#include "defs.h"
#include <assert.h>

// globals
Node* sentinel;
TreeType treeType;

Node* BT_Build(int preorder[], int inorder[], int ilower, int iupper, int index) {
	if (ilower > iupper) return NULL;

	int root = preorder[index];

	int iindex = ilower;
	while (iindex <= iupper && inorder[iindex] != root) ++iindex;

	int leftSize = iindex - ilower;

	Node* node = Node_Alloc(root, 2);
	node->children[0] = BT_Build(preorder, inorder, ilower, iindex - 1, index + 1);
	node->children[1] = BT_Build(preorder, inorder, iindex + 1, iupper, index + 1 + leftSize);
	if (node->children[0]) node->children[0]->par = node;
	if (node->children[1]) node->children[1]->par = node;
	return node;
}

BT BT_Init(int preorder[], int inorder[], size_t len) {
	assert(len > 0);
	treeType = BINARY;

	BT bt = { BT_Build(preorder, inorder, 0, len - 1, 0), len };
	return bt;
}

size_t BT_MaxDepth(BT* tree) {
	if (!tree || !tree->root) return 0;

	size_t maxDepth = 0;
	struct AQueue depth = AQ_Init(tree->size);
	NQueue queue = NQueue_Init(tree->size / 2);
	NQueue_Enqueue(tree->root, &queue);
	AQ_Enqueue(1, &depth);

	while (!NQueue_IsEmpty(&queue)) {
		int dep = AQ_Dequeue(&depth);
		Node* node = NQueue_Dequeue(&queue);

		if (node->children[0]) { NQueue_Enqueue(node->children[0], &queue); AQ_Enqueue(dep + 1, &depth); }
		if (node->children[1]) { NQueue_Enqueue(node->children[1], &queue); AQ_Enqueue(dep + 1, &depth); }
		if (dep > maxDepth) maxDepth = dep;
	}
	return maxDepth;
}

void BT_Add(int val, BT* tree) {
	Node* node = Node_Alloc(val, 2);
	node->children[0] = tree->root;
	if (tree->root) tree->root->par = node;
	tree->root = node;
	tree->size++;
}

void BT_Transplant(Node* root, Node* node, Node* newNode) {
	if (node == root) { root = newNode; return; }
	if (node->par->children[0] == node) node->par->children[0] = newNode;
	else                                 node->par->children[1] = newNode;
	if (newNode) newNode->par = node->par;
}

void BT_Del(Node* node, BT* tree) {
	if (!node || !tree->root) return;

	Node* right = tree->root;
	while (right->children[1]) right = right->children[1];

	BT_Transplant(tree->root, right, right->children[0]);

	if (right == node) { Node_Free(right); return; }

	right->children[0] = node->children[0];
	right->children[1] = node->children[1];
	if (node->children[0]) node->children[0]->par = right;
	if (node->children[1]) node->children[1]->par = right;

	BT_Transplant(tree->root, node, right);
	Node_Free(node);
	tree->size--;
}

void BT_DelByVal(int val, BT* tree) {
	NList queue = NList_Init(tree->size / 2 + 1);
	NList_Add(tree->root, &queue);

	while (queue.len > 0) {
		Node* curr = queue.arr[--queue.len];
		if (!curr) continue;
		if (val == curr->val) { BT_Del(curr, tree); return; }
		NList_Add(curr->children[0], &queue);
		NList_Add(curr->children[1], &queue);
	}
}

Node* BST_Build(int preorder[], int index, int upper) {
	if (index > upper) return NULL;

	int root = preorder[index];

	Node* node = Node_Alloc(root, 2);

	int rightChildIndex = index + 1;
	while (rightChildIndex <= upper && preorder[rightChildIndex] <= root)
		rightChildIndex++;

	node->children[0] = BST_Build(preorder, index + 1, rightChildIndex - 1);
	node->children[1] = BST_Build(preorder, rightChildIndex, upper);
	if (node->children[0]) node->children[0]->par = node;
	if (node->children[1]) node->children[1]->par = node;
	return node;
}

BT BST_Init(int preorder[], size_t len) {
	treeType = BINARY_SEARCH;
	BT tree = { BST_Build(preorder, 0, len - 1), len };
	return tree;
}

Node* BST_Search(int val, BT* bt) {
	Node* temp = bt->root;
	while (temp) {
		if (temp->val > val) temp = temp->children[0];
		else if (temp->val < val) temp = temp->children[1];
		else                      return temp;
	}
	return NULL;
}

void BST_Add(int val, BT* bt) {
	Node* temp = bt->root;
	Node* par = NULL;

	while (temp) {
		par = temp;
		if (temp->val >= val && temp->children[0])      temp = temp->children[0];
		else if (temp->val < val && temp->children[1])  temp = temp->children[1];
		else break;
	}

	Node* node = Node_Alloc(val, 2);
	node->par = par;

	if (!par) { bt->root = node; return; }
	if (par->val >= val) par->children[0] = node;
	else                 par->children[1] = node;
}

void BST_Del(Node* node, BT* bt) {
	if (!node) return;

	if (!node->children[0] && !node->children[1]) {
		if (node == bt->root) bt->root = NULL;
		else {
			if (node->par->children[0] == node) node->par->children[0] = NULL;
			else                                 node->par->children[1] = NULL;
		}
		Node_Free(node);
		return;
	}
	if (!node->children[0] || !node->children[1]) {
		Node* child = node->children[0] ? node->children[0] : node->children[1];
		BT_Transplant(bt->root, node, child);
		Node_Free(node);
		return;
	}

	// find in-order successor (leftmost in right subtree)
	Node* succ = node->children[1];
	while (succ->children[0]) succ = succ->children[0];

	if (succ->par != node) {
		BT_Transplant(bt->root, succ, succ->children[1]);
		succ->children[1] = node->children[1];
		succ->children[1]->par = succ;
	}
	BT_Transplant(bt->root, node, succ);
	succ->children[0] = node->children[0];
	succ->children[0]->par = succ;
	Node_Free(node);
}

uint16_t BT_Depth(const Node* root) {
	if (!root) return 0;

	uint16_t depth = 0;
	NList q = NList_Init(1);
	NList_Add((Node*)root, &q);

	size_t head = 0;
	while (head < q.len) {
		size_t end = q.len;
		while (head < end) {
			Node* n = q.arr[head++];
			if (n->children[0]) NList_Add(n->children[0], &q);
			if (n->children[1]) NList_Add(n->children[1], &q);
		}
		depth++;
	}

	free(q.arr);
	return depth;
}

Node* RB_NodeAlloc(int val) {
	Node* node = Node_Alloc(val, 2);
	node->color = RED;
	node->par = sentinel;
	node->children[0] = sentinel;
	node->children[1] = sentinel;

	return node;
}

BT RB_Init(int preorder[], size_t len)
{
	assert(len != 0);
	treeType = RED_BLACK;

	sentinel = RB_NodeAlloc(INT32_MIN);
	RB_ResetSentinel();

	BT bt = (BT){
		sentinel, 1
	};

	for (int i = 0; i < len; i++) {
		RB_Add(preorder[i], &bt);
	}

	return bt;
}

void RB_LeftRotate(Node* node, BT* bt) {

	Node* right = node->children[1];
	node->children[1] = right->children[0];

	if (node->children[1] != sentinel) {
		node->children[1]->par = node;
	}
	right->par = node->par;

	if (node->par == sentinel) {
		bt->root = right;
	}
	else if (node->par->children[0] == node) {
		node->par->children[0] = right;
	}
	else {
		node->par->children[1] = right;
	}

	right->children[0] = node;

	node->par = right;
}

void RB_ResetSentinel()
{
	sentinel->color = GRAY;
	sentinel->par = sentinel;
	sentinel->children[0] = sentinel;
	sentinel->children[1] = sentinel;
	sentinel->val = INT32_MIN;
}

void RB_RightRotate(Node* node, BT* bt) {
	Node* left = node->children[0];
	node->children[0] = left->children[1];

	if (node->children[0] != sentinel) {
		node->children[0]->par = node;
	}
	left->par = node->par;

	if (node->par == sentinel) {
		bt->root = left;
	}
	else if (node->par->children[0] == node) {
		node->par->children[0] = left;
	}
	else {
		node->par->children[1] = left;
	}

	left->children[1] = node;

	node->par = left;
}
void RB_AddFixup(Node* node, BT* bt) {
	if (node == sentinel) return;

	while (node->par->color == RED && node != sentinel && node->par != sentinel) {
		Node* parent = node->par;

		if (parent == parent->par->children[0]) {
			Node* uncle = parent->par->children[1];

			if (uncle->color == RED) {
				uncle->color = GRAY;
				parent->color = GRAY;
				parent->par->color = RED;
				node = parent->par;
			}
			else {
				if (node == parent->children[1]) {
					node = parent;
					RB_LeftRotate(node, bt);
				}
				node->par->color = GRAY;
				node->par->par->color = RED;

				RB_RightRotate(node->par->par, bt);
			}
		}
		else {
			Node* uncle = parent->par->children[0];

			if (uncle->color == RED) {
				uncle->color = GRAY;
				parent->color = GRAY;
				parent->par->color = RED;
				node = parent->par;
			}
			else {
				if (node == parent->children[0]) {
					node = parent;
					RB_RightRotate(node, bt);
				}
				node->par->color = GRAY;
				node->par->par->color = RED;

				RB_LeftRotate(node->par->par, bt);
			}
		}

		RB_ResetSentinel();
	}

	bt->root->color = GRAY;
	sentinel->color = GRAY;
	sentinel->children[0] = sentinel;
	sentinel->children[1] = sentinel;
	sentinel->par = sentinel;
}

void RB_Add(int val, BT* bt)
{
	Node* node = RB_NodeAlloc(val);

	Node* dest = sentinel;
	Node* temp = bt->root;

	while (temp != sentinel) {
		dest = temp;

		if (temp->val < val) {
			temp = temp->children[1];
		}
		else {
			temp = temp->children[0];
		}
	}

	node->par = dest;

	if (dest == sentinel) {
		bt->root = node;
	}
	else if (dest->val < val) {
		// new right child
		dest->children[1] = node;
	}
	else {
		// new left child
		dest->children[0] = node;
	}

	node->color = RED;
	RB_AddFixup(node, bt);
	bt->size++;
}

void RB_Transplant(Node* oldNode, Node* newNode, BT* bt) {
	if (oldNode->par == sentinel) {
		bt->root = newNode;
	}
	else if (oldNode == oldNode->par->children[0]) {
		oldNode->par->children[0] = newNode;
	}
	else {
		oldNode->par->children[1] = newNode;
	}
	newNode->par = oldNode->par;
}

void RB_DelFixup(Node* fixupNode, BT* bt)
{

	while (fixupNode != bt->root && fixupNode->color == BLACK) {
		Node* parent = fixupNode->par;
		Node* temp;

		if (fixupNode == node_left(parent)) {
			temp = node_right(parent);

			if (temp->color == RED) {
				// case 1
				temp->color = BLACK;
				parent->color = RED;
				RB_LeftRotate(parent, bt);
				temp = node_right(parent);
			}

			if (node_left(temp)->color == BLACK && node_right(temp)->color == BLACK) {
				// case 2
				temp->color = RED;
				fixupNode = fixupNode->par;
			}
			else {
				if (node_left(temp)->color == RED) {
					// case 3
					temp->color = RED;
					node_left(temp)->color = BLACK;
					RB_RightRotate(temp, bt);
				}

				temp->color = parent->color;
				parent->color = BLACK;
				RB_LeftRotate(parent, bt);
			}
		}
		else {
			// just swap left with right
			temp = node_left(parent);

			if (temp->color == RED) {
				// case 1
				temp->color = BLACK;
				parent->color = RED;
				RB_RightRotate(parent, bt);
				temp = node_left(parent);
			}

			if (node_left(temp)->color == BLACK && node_right(temp)->color == BLACK) {
				// case 2
				temp->color = RED;
				fixupNode = fixupNode->par;
			}
			else {
				if (node_right(temp)->color == RED) {
					// case 3
					temp->color = RED;
					node_right(temp)->color = BLACK;
					RB_LeftRotate(temp, bt);
				}

				temp->color = parent->color;
				parent->color = BLACK;
				RB_RightRotate(parent, bt);
			}
		}
	}

	fixupNode->color = BLACK;
}

void RB_Del(Node* node, BT* bt)
{
	assert(node);
	bt->size--;
	COLOR originalColor = node->color;
	Node* fixupNode = NULL;

	if (node->children[0] == sentinel) {
		fixupNode = node->children[1];
		RB_Transplant(node, fixupNode, bt);
	}
	else if (node->children[1] = sentinel) {
		fixupNode = node->children[0];
		RB_Transplant(node, fixupNode, bt);
	}
	else {
		Node* temp = node_right(node);
		if (temp == sentinel) {
			return;
		}

		while (node_left(temp) != sentinel) {
			temp = node_left(temp);
		}

		originalColor = temp->color;
		fixupNode = node_right(temp);

		if (temp->par == node) {
			// for case when fixupNode == sentinel
			fixupNode->par = temp;
		}
		else {
			RB_Transplant(temp, fixupNode, bt);
			node_right(temp) = node_right(node);
			node_right(temp)->par = temp;
		}

		RB_Transplant(node, temp, bt);
		node_left(temp) = node_left(node);
		if (node_left(temp)) node_left(node)->par = temp;
		temp->color = node->color;
	}

	if (originalColor == BLACK) {
		RB_DelFixup(fixupNode, bt);
	}

}

void BT_PrintFormatRows(const Node* root, uint16_t depth) {
}

void BT_Print(const BT* bt) {
	uint16_t depth = BT_Depth(bt->root);
	BT_PrintFormatRows(bt->root, depth);
}
