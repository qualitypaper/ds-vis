#include "bt.h"
#include "queue.h"

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
	BT bt = { BT_Build(preorder, inorder, 0, len - 1, 0), len };
	return bt;
}

size_t BT_MaxDepth(struct BT* tree) {
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

void BT_Add(int val, struct BT* tree) {
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

void BT_Del(Node* node, struct BT* tree) {
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

void BT_DelByVal(int val, struct BT* tree) {
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

Node* BST_Build(int preorder[], size_t index, size_t upper) {
	if (index > upper) return NULL;

	int root = preorder[index];

	Node* node = Node_Alloc(root, 2);

	size_t rightChildIndex = index + 1;
	while (rightChildIndex <= upper && preorder[rightChildIndex] <= root)
		rightChildIndex++;

	node->children[0] = BST_Build(preorder, index + 1, rightChildIndex - 1);
	node->children[1] = BST_Build(preorder, rightChildIndex, upper);
	if (node->children[0]) node->children[0]->par = node;
	if (node->children[1]) node->children[1]->par = node;
	return node;
}

struct BT BST_Init(int preorder[], size_t len) {
	struct BT tree = { BST_Build(preorder, 0, len - 1), len };
	return tree;
}

Node* BST_Search(int val, struct BT* bt) {
	Node* temp = bt->root;
	while (temp) {
		if (temp->val > val) temp = temp->children[0];
		else if (temp->val < val) temp = temp->children[1];
		else                      return temp;
	}
	return NULL;
}

void BST_Add(int val, struct BT* bt) {
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

void BST_Del(Node* node, struct BT* bt) {
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

void BT_PrintFormatRows(const Node* root, uint16_t depth) {
}

void BT_Print(const struct BT* bt) {
	uint16_t depth = BT_Depth(bt->root);
	BT_PrintFormatRows(bt->root, depth);
}
