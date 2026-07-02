#include "bt.h"
#include "queue.h"

struct Node* BT_Build(int preorder[], int inorder[], int ilower, int iupper, int index) {
	if (ilower > iupper) return NULL;

	int root = preorder[index];

	int iindex = ilower;
	while (iindex <= iupper && inorder[iindex] != root) ++iindex;

	int leftSize = iindex - ilower;

	struct Node* node = (struct Node*)malloc(sizeof(struct Node));
	node->left = BT_Build(preorder, inorder, ilower, iindex - 1, index + 1);
	node->right = BT_Build(preorder, inorder, iindex + 1, iupper, index + 1 + leftSize);
	if (node->left)  node->left->par = node;
	if (node->right) node->right->par = node;
	node->val = root;
	node->par = NULL;

	return node;
}

struct BT BT_Init(int preorder[], int inorder[], size_t len) {
	struct BT bt = { BT_Build(preorder, inorder, 0, len-1, 0), len };
	return bt;
}

size_t BT_MaxDepth(struct BT* tree) {
	if (!tree || !tree->root) return 0;

	size_t maxDepth = 0;
	struct AQueue depth = AQ_Init(tree->size);
	struct NQueue queue = NQueue_Init(tree->size/2);
	NQueue_Enqueue(tree->root, &queue);
	AQ_Enqueue(1, &depth);

	while (!NQueue_IsEmpty(&queue)) {
		int dep = AQ_Dequeue(&depth);
		struct Node* node = NQueue_Dequeue(&queue);

		if (node->left) {
			NQueue_Enqueue(node->left, &queue);
			AQ_Enqueue(dep + 1, &depth);
		}

		if (node->right) {
			NQueue_Enqueue(node->right, &queue);
			AQ_Enqueue(dep + 1, &depth);
		}

		if (dep > maxDepth) {
			maxDepth = dep;
		}
	}

	return maxDepth;
}

void BT_Add(int val, struct BT* tree) {
	struct Node* node = (struct Node*)malloc(sizeof(struct Node));
	node->left = tree->root;
	node->right = NULL;
	node->par = NULL;
	node->val = val;
	if (tree->root) tree->root->par = node;
	tree->root = node;
	tree->size++;
}

void BT_Transplant(struct Node* root, struct Node* node, struct Node* newNode) {
	if (node == root) { root = newNode; return; }
	if (node->par->left == node) node->par->left = newNode;
	else                         node->par->right = newNode;
	if (newNode) newNode->par = node->par;
}

void BT_Del(struct Node* node, struct BT* tree) {
	if (!node || !tree->root) return;

	struct Node* right = tree->root;
	while (right->right) right = right->right;

	BT_Transplant(tree->root, right, right->left);

	if (right == node) { free(right); return; }

	right->left = node->left;
	right->right = node->right;
	if (node->left)  node->left->par = right;
	if (node->right) node->right->par = right;

	BT_Transplant(tree->root, node, right);
	free(node);
	tree->size--;
}

void BT_DelByVal(int val, struct BT* tree) {
	struct NList queue = NList_Init(tree->size / 2 + 1);
	NList_Add(tree->root, &queue);

	while (queue.len > 0) {
		struct Node* curr = queue.arr[--queue.len];
		if (!curr) continue;
		if (val == curr->val) { BT_Del(curr, tree); return; }
		NList_Add(curr->left, &queue);
		NList_Add(curr->right, &queue);
	}
}

struct Node* BST_Build(int preorder[], size_t index, size_t upper) {
	if (index > upper) return NULL;

	int root = preorder[index];

	struct Node* node = (struct Node*)malloc(sizeof(struct Node));
	node->val = root;
	node->par = NULL;

	size_t rightChildIndex = index + 1;
	while (rightChildIndex <= upper && preorder[rightChildIndex] <= root)
		rightChildIndex++;

	node->left = BST_Build(preorder, index + 1, rightChildIndex - 1);
	node->right = BST_Build(preorder, rightChildIndex, upper);
	if (node->left)  node->left->par = node;
	if (node->right) node->right->par = node;
	return node;
}

struct BT BST_Init(int preorder[], size_t len) {
	struct BT tree = { BST_Build(preorder, 0, len - 1), len };
	return tree;
}

struct Node* BST_Search(int val, struct BT* bt) {
	struct Node* temp = bt->root;
	while (temp) {
		if (temp->val > val) temp = temp->left;
		else if (temp->val < val) temp = temp->right;
		else                      return temp;
	}
	return NULL;
}

void BST_Add(int val, struct BT* bt) {
	struct Node* temp = bt->root;
	struct Node* par = NULL;

	while (temp) {
		par = temp;
		if (temp->val >= val && temp->left)  temp = temp->left;
		else if (temp->val < val && temp->right) temp = temp->right;
		else break;
	}

	struct Node* node = (struct Node*)malloc(sizeof(struct Node));
	node->left = NULL;
	node->right = NULL;
	node->par = par;
	node->val = val;

	if (!par) { bt->root = node; return; }
	if (par->val >= val) par->left = node;
	else                 par->right = node;
}

void BST_Del(struct Node* node, struct BT* bt) {
	if (!node) return;

	if (!node->left && !node->right) {
		if (node == bt->root) bt->root = NULL;
		else { if (node->par->left == node) node->par->left = NULL; else node->par->right = NULL; }
		free(node);
		return;
	}
	if (!node->left || !node->right) {
		struct Node* child = node->left ? node->left : node->right;
		BT_Transplant(bt->root, node, child);
		free(node);
		return;
	}

	// find in-order successor (leftmost in right subtree)
	struct Node* succ = node->right;
	while (succ->left) succ = succ->left;

	if (succ->par != node) {
		BT_Transplant(bt->root, succ, succ->right);
		succ->right = node->right;
		succ->right->par = succ;
	}
	BT_Transplant(bt->root, node, succ);
	succ->left = node->left;
	succ->left->par = succ;
	free(node);
}


uint16_t BT_Depth(const struct Node* root) {
	if (!root) return 0;

	uint16_t depth = 0;
	struct NList q = NList_Init(1);
	NList_Add((struct Node*)root, &q);

	size_t head = 0;
	while (head < q.len) {
		size_t end = q.len;
		while (head < end) {
			struct Node* n = q.arr[head++];
			if (n->left)  NList_Add(n->left, &q);
			if (n->right) NList_Add(n->right, &q);
		}
		depth++;
	}

	free(q.arr);
	return depth;
}

void BT_PrintFormatRows(const struct Node* root, uint16_t depth) {
	
}

void BT_Print(const struct BT* bt) {
	uint16_t depth = BT_Depth(bt->root);

	BT_PrintFormatRows(bt->root, depth);
}