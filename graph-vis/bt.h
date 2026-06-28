#ifndef GRAPH_VIS_DEFS
#define GRAPH_VIS_DEFS

#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct Node {
	struct Node* left;
	struct Node* right;
	struct Node* par;

	int val;
};

DEFINE_LIST(struct Node*, NList)

struct BT {
	struct Node* root;
	size_t size;
};

struct BT BT_Init(int preorder[], int inorder[], size_t len) {
	struct BT bt = {
		BT_Build(preorder, inorder, 0, len - 1, 0),
		len
	};

	return bt;
}

struct Node* BT_Build(int preorder[], int inorder[], size_t ilower, size_t iupper, size_t index) {
	if (ilower > iupper) return NULL;

	int root = preorder[index];

	size_t iindex = ilower;
	while (inorder[iindex] != root) {
		++iindex;
	}

	size_t leftSize = min(0, iindex - ilower);

	struct Node* node = malloc(sizeof(struct Node));

	node->left = BT_Build(preorder, inorder, ilower, iindex - 1, index + 1);
	if (node->left) node->left->par = node;

	node->right = BT_Build(preorder, inorder, iindex + 1, iupper, index + 1 + leftSize);
	if (node->right) node->right->par = node;

	node->val = root;

	return node;
}

void BT_Add(int val, struct BT* tree) {
	struct Node* node = malloc(sizeof(struct Node));

	node->left = tree->root;
	tree->root->par = node;

	node->par = NULL;
	node->right = NULL;
	node->val = val;

	tree->root = node;
	tree->size++;
}

void BT_Del(struct Node* node, struct BT* tree) {
	if (!node) return;

	struct Node* rightPar = tree->root;
	struct Node* right = tree->root;
	while (right->right) {
		rightPar = right;
		right = right->right;
	}

	BT_Transplant(tree->root, right, right->left);

	if (right == node) {
		free(right);
		return;
	}

	right->left = node->left;
	if (node->left)
		right->left->par = node->left->par;

	right->right = node->right;
	if (node->right)
		right->right->par = node->right->par;

	BT_Transplant(tree->root, node, right);

	free(node);
	tree->size--;
}

void BT_DelByVal(int val, struct BT* tree) {

	struct NList queue = NList_Init(tree->size / 1.6);

	NList_Add(tree->root, &queue);

	while (queue.len > 0) {
		struct Node* curr = queue.arr[queue.len];

		if (val == curr->val) {
			BT_Del(curr, tree);
			return;
		}

		NList_Add(curr->left, &queue);
		NList_Add(curr->right, &queue);
	}
}


void BT_Transplant(struct Node* root, struct Node* node, struct Node* newNode) {
	if (node == root) {
		root = newNode;
		return;
	}

	if (node->par->left == node) {
		node->par->left = newNode;
	}
	else {
		node->par->right = newNode;
	}

	if (newNode)
		newNode->par = node->par;
}

struct BT BST_Init(int preorder[], size_t len) {
	struct BT tree = {
		BST_Build(preorder, 0, len - 1),
		len
	};
	return tree;
}

struct Node* BST_Build(int preorder[], size_t index, size_t upper) {
	if (index > upper)
		return NULL;

	int root = preorder[index];

	struct Node* node = malloc(sizeof(struct Node));

	size_t rightChildIndex = index + 1;

	while (preorder[rightChildIndex] <= root) {
		rightChildIndex++;
	}

	node->left = BST_Build(preorder, index + 1, rightChildIndex);
	if (node->left) node->left->par = node;

	node->right = BST_Build(preorder, rightChildIndex, upper);
	if (node->right) node->right->par = node;

	node->val = root;

	return node;
}

struct Node* BST_Search(int val, struct BT* bt) {
	struct Node* temp = bt->root;

	while (temp) {
		if (temp->val > val) temp = temp->left;
		else if (temp->val < val) temp = temp->right;
		else return temp;
	}

	return NULL;
}

void BST_Add(int val, struct BT* bt) {
	struct Node* temp = bt->root;
	struct Node* par = NULL;

	while (temp) {
		par = temp;
		if (temp->val >= val && temp->left) {
			temp = temp->left;
		}
		else if (temp->val < val && temp->right) {
			temp = temp->right;
		}
		else {
			break;
		}
	}

	struct Node* node = malloc(sizeof(struct Node));
	node->left = NULL;
	node->right = NULL;
	node->par = par;
	node->val = val;

	if (par->val >= val)
		par->left = node;
	else
		par->right = node;

	if (!bt->root) {
		bt->root = node;
	}
}

void BST_Del(struct Node* node, struct BT* bt) {
	if (!node) return;
	else if (!node->left && !node->right) {
		node->par = NULL;
		if (node == bt->root)
			bt->root = NULL;

		free(node);
		return;
	}
	else if (!node->left ^ !node->right) {
		BT_Transplant(bt->root, node, node->left ? node->left : node->right);
		free(node);
		return;
	}
	else if (node->right && !node->right->left) {
		BT_Transplant(bt->root, node, node->right);
		node->right->left = node->left;
		if (node->left) node->left->par = node->right;

		free(node);
		return;
	}
	else if (node->left && !node->left->right) {
		BT_Transplant(bt->root, node, node->left);
		node->left->right = node->right;
		if (node->right) node->right->par = node->left;

		free(node);
		return;
	}

	struct Node* temp = node->right;

	while (temp->left) {
		temp = temp->left;
	}

	if (temp->par != node) {
		BT_Transplant(bt->root, temp, temp->right);
		temp->right = node->right;
		node->right->par = temp;
	}

	BT_Transplant(bt->root, node, temp);
	temp->left = node->left;
	node->left->par = temp->left;

	free(node);
}

#endif