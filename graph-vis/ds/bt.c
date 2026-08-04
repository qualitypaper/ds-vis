#include "bt.h"
#include "defs.h"
#include "queue.h"
#include "engine/tree_render.h"
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

// globals
Node *sentinel = NULL;
TreeType treeType;

static Node *Sentinel_Alloc(void) {
    if (sentinel) return sentinel;
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t pageSize = si.dwPageSize;
    void *page = VirtualAlloc(NULL, pageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    assert(page != NULL);
#else
    long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize <= 0) pageSize = 4096;
    void *page = mmap(NULL, (size_t) pageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(page != MAP_FAILED);
#endif

    Node *s = (Node *) page;
    Node **children = (Node **) ((char *) page + sizeof(Node));

    s->id = (ID) s;
    s->children = children;
    s->childCount = 2;
    s->par = s;
    s->val = INT32_MIN;
    s->height = -1;
    s->color = GRAY;

    children[0] = s;
    children[1] = s;

#ifdef _WIN32
    DWORD oldProtect;
    VirtualProtect(page, pageSize, PAGE_READONLY, &oldProtect);
#else
    mprotect(page, (size_t) pageSize, PROT_READ);
#endif

    return s;
}

static int IsValid(const Node *node) {
    if (!node || node == sentinel) return 0;
    if (node->par && node->par != sentinel) {
        if (node->par->children[0] != node && node->par->children[1] != node) return 0;
    }
    return 1;
}

Node *BT_Build(int preorder[], int inorder[], int ilower, int iupper, int index) {
    if (ilower > iupper) return NULL;

    int root = preorder[index];

    int iindex = ilower;
    while (iindex <= iupper && inorder[iindex] != root) ++iindex;

    int leftSize = iindex - ilower;

    Node *node = Node_Alloc(root, 2);
    node->children[0] = BT_Build(preorder, inorder, ilower, iindex - 1, index + 1);
    node->children[1] = BT_Build(preorder, inorder, iindex + 1, iupper, index + 1 + leftSize);
    if (node->children[0]) node->children[0]->par = node;
    if (node->children[1]) node->children[1]->par = node;
    return node;
}

BT BT_Init(int preorder[], int inorder[], size_t len) {
    assert(len > 0);
    treeType = BINARY;

    BT bt = {.type = BINARY, .root = BT_Build(preorder, inorder, 0, len - 1, 0), .size = len};
    return bt;
}

size_t BT_MaxDepth(BT *tree) {
    if (!tree || !tree->root) return 0;

    size_t maxDepth = 0;
    struct AQueue depth = AQ_Init(tree->size);
    NQueue queue = NQueue_Init(tree->size / 2);
    NQueue_Enqueue(tree->root, &queue);
    AQ_Enqueue(1, &depth);

    while (!NQueue_IsEmpty(&queue)) {
        int dep = AQ_Dequeue(&depth);
        Node *node = NQueue_Dequeue(&queue);

        if (node->children[0]) {
            NQueue_Enqueue(node->children[0], &queue);
            AQ_Enqueue(dep + 1, &depth);
        }
        if (node->children[1]) {
            NQueue_Enqueue(node->children[1], &queue);
            AQ_Enqueue(dep + 1, &depth);
        }
        if (dep > maxDepth) maxDepth = dep;
    }
    return maxDepth;
}

void BT_Add(int val, BT *tree) {
    Node *node = Node_Alloc(val, 2);
    node->children[0] = tree->root;
    if (tree->root) tree->root->par = node;
    tree->root = node;
    tree->size++;
}

void BT_Transplant(Node *root, Node *node, Node *newNode) {
    if (node == root) {
        root = newNode;
        return;
    }
    if (node->par->children[0] == node) node->par->children[0] = newNode;
    else node->par->children[1] = newNode;
    if (newNode) newNode->par = node->par;
}

void BT_Del(Node *node, BT *tree) {
    if (!node || !tree->root) return;

    Node *right = tree->root;
    while (right->children[1]) right = right->children[1];

    BT_Transplant(tree->root, right, right->children[0]);

    if (right == node) {
        Node_Free(right);
        return;
    }

    right->children[0] = node->children[0];
    right->children[1] = node->children[1];
    if (node->children[0]) node->children[0]->par = right;
    if (node->children[1]) node->children[1]->par = right;

    BT_Transplant(tree->root, node, right);
    Node_Free(node);
    tree->size--;
}

void BT_DelByVal(int val, BT *tree) {
    NList queue = NList_Init(tree->size / 2 + 1);
    NList_Add(tree->root, &queue);

    while (queue.len > 0) {
        Node *curr = queue.arr[--queue.len];
        if (!curr) continue;
        if (val == curr->val) {
            BT_Del(curr, tree);
            return;
        }
        NList_Add(curr->children[0], &queue);
        NList_Add(curr->children[1], &queue);
    }
}

Node *BST_Build(int preorder[], int index, int upper) {
    if (index > upper) return NULL;

    int root = preorder[index];

    Node *node = Node_Alloc(root, 2);

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
    treeType = BST;
    BT tree = {BST, BST_Build(preorder, 0, len - 1), len};
    return tree;
}

Node *BST_Search(int val, BT *bt) {
    Node *temp = bt->root;
    while (temp) {
        if (temp->val > val) temp = temp->children[0];
        else if (temp->val < val) temp = temp->children[1];
        else return temp;
    }
    return NULL;
}

void BST_Add(int val, BT *bt) {
    Node *temp = bt->root;
    Node *par = NULL;

    while (temp) {
        par = temp;
        if (temp->val >= val && temp->children[0]) temp = temp->children[0];
        else if (temp->val < val && temp->children[1]) temp = temp->children[1];
        else break;
    }

    Node *node = Node_Alloc(val, 2);
    node->par = par;

    if (!par) {
        bt->root = node;
        return;
    }
    if (par->val >= val) par->children[0] = node;
    else par->children[1] = node;
}

void BST_Del(Node *node, BT *bt) {
    if (!node) return;

    if (!node->children[0] && !node->children[1]) {
        if (node == bt->root) bt->root = NULL;
        else {
            if (node->par->children[0] == node) node->par->children[0] = NULL;
            else node->par->children[1] = NULL;
        }
        Node_Free(node);
        return;
    }
    if (!node->children[0] || !node->children[1]) {
        Node *child = node->children[0] ? node->children[0] : node->children[1];
        BT_Transplant(bt->root, node, child);
        Node_Free(node);
        return;
    }

    // find in-order successor (leftmost in right subtree)
    Node *succ = node->children[1];
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

uint16_t BT_Depth(const Node *root) {
    if (!root) return 0;

    uint16_t depth = 0;
    NList q = NList_Init(1);
    NList_Add((Node *) root, &q);

    size_t head = 0;
    while (head < q.len) {
        size_t end = q.len;
        while (head < end) {
            Node *n = q.arr[head++];
            if (n->children[0]) NList_Add(n->children[0], &q);
            if (n->children[1]) NList_Add(n->children[1], &q);
        }
        depth++;
    }

    free(q.arr);
    return depth;
}

Node *RB_NodeAlloc(int val) {
    if (!sentinel) sentinel = Sentinel_Alloc();
    Node *node = Node_Alloc(val, 2);
    node->color = RED;
    node->par = sentinel;
    node->children[0] = sentinel;
    node->children[1] = sentinel;

    return node;
}

BT RB_Init(int preorder[], size_t len) {
    assert(len != 0);
    treeType = RED_BLACK;

    if (!sentinel) sentinel = Sentinel_Alloc();

    BT bt = (BT){
        .type = RED_BLACK,
        .root = sentinel,
        .size = 0
    };

    for (size_t i = 0; i < len; i++) {
        RB_Add(preorder[i], &bt);
    }

    return bt;
}

void RB_LeftRotate(Node *node, BT *bt) {
    if (!node || node == sentinel) return;
    Node *right = node_right(node);
    if (!right || right == sentinel) return;

    node_right(node) = node_left(right);

    if (node_right(node) != sentinel) {
        node_right(node)->par = node;
    }

    right->par = node->par;

    if (node->par == sentinel) {
        bt->root = right;
    } else if (node->par->children[0] == node) {
        node->par->children[0] = right;
    } else {
        node->par->children[1] = right;
    }

    node_left(right) = node;
    node->par = right;
}

void RB_RightRotate(Node *node, BT *bt) {
    if (!node || node == sentinel) return;
    Node *left = node_left(node);
    if (!left || left == sentinel) return;

    node_left(node) = node_right(left);

    if (node_left(node) != sentinel) {
        node_left(node)->par = node;
    }

    left->par = node->par;

    if (node->par == sentinel) {
        bt->root = left;
    } else if (node->par->children[0] == node) {
        node->par->children[0] = left;
    } else {
        node->par->children[1] = left;
    }

    node_right(left) = node;
    node->par = left;
}

void RB_AddFixup(Node *node, BT *bt) {
    if (node == sentinel) return;

    while (node->par->color == RED && node != sentinel && node->par != sentinel) {
        Node *parent = node->par;

        if (parent == parent->par->children[0]) {
            Node *uncle = parent->par->children[1];

            if (uncle->color == RED) {
                uncle->color = GRAY;
                parent->color = GRAY;
                parent->par->color = RED;
                node = parent->par;
            } else {
                if (node == parent->children[1]) {
                    node = parent;
                    RB_LeftRotate(node, bt);
                }
                node->par->color = GRAY;
                node->par->par->color = RED;

                RB_RightRotate(node->par->par, bt);
            }
        } else {
            Node *uncle = parent->par->children[0];

            if (uncle->color == RED) {
                uncle->color = GRAY;
                parent->color = GRAY;
                parent->par->color = RED;
                node = parent->par;
            } else {
                if (node == parent->children[0]) {
                    node = parent;
                    RB_RightRotate(node, bt);
                }
                node->par->color = GRAY;
                node->par->par->color = RED;

                RB_LeftRotate(node->par->par, bt);
            }
        }
    }

    if (bt->root && bt->root != sentinel) {
        bt->root->color = GRAY;
    }
}

void RB_Add(int val, BT *bt) {
    Node *node = RB_NodeAlloc(val);

    Node *dest = sentinel;
    Node *temp = bt->root;

    while (temp != sentinel) {
        dest = temp;

        if (temp->val < val) {
            temp = temp->children[1];
        } else {
            temp = temp->children[0];
        }
    }

    node->par = dest;

    if (dest == sentinel) {
        bt->root = node;
    } else if (dest->val < val) {
        // new right child
        dest->children[1] = node;
    } else {
        // new left child
        dest->children[0] = node;
    }

    node->color = RED;
    RB_AddFixup(node, bt);
    bt->size++;
}

void RB_Transplant(Node *oldNode, Node *newNode, BT *bt) {
    if (oldNode->par == sentinel) {
        bt->root = newNode;
    } else if (oldNode == oldNode->par->children[0]) {
        oldNode->par->children[0] = newNode;
    } else {
        oldNode->par->children[1] = newNode;
    }
    if (newNode != sentinel) {
        newNode->par = oldNode->par;
    }
}

void RB_DelFixup(Node *fixupNode, Node *parent, BT *bt) {
    if (!fixupNode) return;

    while (fixupNode != bt->root && fixupNode->color == BLACK) {
        Node *p = (fixupNode != sentinel) ? fixupNode->par : parent;
        if (!p || p == sentinel) break;

        Node *temp;

        if (fixupNode == node_left(p)) {
            temp = node_right(p);

            if (temp->color == RED) {
                // case 1
                temp->color = BLACK;
                p->color = RED;
                RB_LeftRotate(p, bt);
                p = (fixupNode != sentinel) ? fixupNode->par : parent;
                temp = node_right(p);
            }

            if (node_left(temp)->color == BLACK && node_right(temp)->color == BLACK) {
                // case 2
                temp->color = RED;
                fixupNode = p;
                parent = p->par;
            } else {
                if (node_right(temp)->color == BLACK) {
                    // case 3: near (left) nephew red, far (right) nephew black
                    if (node_left(temp) != sentinel)
                        node_left(temp)->color = BLACK;
                    temp->color = RED;
                    RB_RightRotate(temp, bt);
                    temp = node_right(p); // sibling changed; re-fetch before case 4
                }

                // case 4
                temp->color = p->color;
                p->color = BLACK;
                if (node_right(temp) != sentinel)
                    node_right(temp)->color = BLACK;
                RB_LeftRotate(p, bt);
                fixupNode = bt->root; // fully resolved; end the loop
            }
        } else {
            // just swap left with right
            temp = node_left(p);

            if (temp->color == RED) {
                // case 1
                temp->color = BLACK;
                p->color = RED;
                RB_RightRotate(p, bt);
                p = (fixupNode != sentinel) ? fixupNode->par : parent;
                temp = node_left(p);
            }

            if (node_left(temp)->color == BLACK && node_right(temp)->color == BLACK) {
                // case 2
                temp->color = RED;
                fixupNode = p;
                parent = p->par;
            } else {
                if (node_left(temp)->color == BLACK) {
                    // case 3: near (right) nephew red, far (left) nephew black
                    if (node_right(temp) != sentinel)
                        node_right(temp)->color = BLACK;
                    temp->color = RED;
                    RB_LeftRotate(temp, bt);
                    temp = node_left(p); // sibling changed; re-fetch before case 4
                }

                // case 4
                temp->color = p->color;
                p->color = BLACK;
                if (node_left(temp) != sentinel)
                    node_left(temp)->color = BLACK;
                RB_RightRotate(p, bt);
                fixupNode = bt->root; // fully resolved; end the loop
            }
        }
    }

    if (fixupNode != sentinel) {
        fixupNode->color = BLACK;
    }
}

void RB_Del(Node *node, BT *bt) {
    if (!node || node == sentinel) return;

    bt->size--;
    COLOR originalColor = node->color;
    Node *fixupNode = sentinel;
    Node *parent = sentinel;

    if (node_left(node) == sentinel) {
        fixupNode = node_right(node);
        parent = node->par;
        RB_Transplant(node, fixupNode, bt);
    } else if (node_right(node) == sentinel) {
        fixupNode = node_left(node);
        parent = node->par;
        RB_Transplant(node, fixupNode, bt);
    } else {
        Node *temp = node_right(node);
        if (temp == sentinel) {
            return;
        }

        while (node_left(temp) != sentinel) {
            temp = node_left(temp);
        }

        originalColor = temp->color;
        fixupNode = node_right(temp);

        if (temp->par == node) {
            parent = temp;
            if (fixupNode != sentinel) {
                fixupNode->par = temp;
            }
        } else {
            parent = temp->par;
            RB_Transplant(temp, fixupNode, bt);
            node_right(temp) = node_right(node);
            if (node_right(temp) != sentinel) {
                node_right(temp)->par = temp;
            }
        }

        RB_Transplant(node, temp, bt);
        node_left(temp) = node_left(node);
        if (node_left(temp) != sentinel)
            node_left(temp)->par = temp;
        temp->color = node->color;
    }

    if (originalColor == BLACK) {
        RB_DelFixup(fixupNode, parent, bt);
    }

    Node_Free(node);
}

// -----------------------------------------------------------
//						AVL-Trees
// -----------------------------------------------------------


static int AVL_UpdateBalance(Node *node) {
    if (!IsValid(node)) return 0;

    const int leftHeight = IsValid(node_left(node)) ? node_left(node)->height : 0;
    const int rightHeight = IsValid(node_right(node)) ? node_right(node)->height : 0;

    node->height = max(leftHeight, rightHeight) + 1;
    node->balance = rightHeight - leftHeight;
    return node->balance;
}

void AVL_LeftRotate(Node *node, BT *bt) {
    RB_LeftRotate(node, bt);

    Node* right = node->par;

    right->height = node->height;
    node->height = max(node_left(node)->height, node_right(node)->height) + 1;

    node->balance = node_right(node)->height - node_left(node)->height;
    right->balance = node_right(right)->height - node_left(right)->height;
}

void AVL_RightRotate(Node *node, BT *bt) {
    RB_RightRotate(node, bt);
    Node* left = node->par;

    left->height = node->height;
    node->height = max(node_left(node)->height, node_right(node)->height) + 1;

    node->balance = node_right(node)->height - node_left(node)->height;
    left->balance = node_right(left)->height - node_left(left)->height;
}

static int AVL_GetHeight(Node *node) {
    if (!IsValid(node)) {
        return 0;
    }

    int left = AVL_GetHeight(node_left(node));
    int right = AVL_GetHeight(node_right(node));
    node->height = max(left, right) + 1;
    return node->height;
}

BT AVL_Init(int preorder[], const size_t len) {
    assert(len > 0);

    BT bt = (BT){
        .type = AVL,
        .root = NULL,
        .size = 0
    };
    treeType = AVL;

    if (!sentinel) sentinel = Sentinel_Alloc();

    for (size_t i = 0; i < len; i++) {
        AVL_Add(preorder[i], &bt);
    }

    return bt;
}

int AVL_GetBalance(Node *node) {
    if (!IsValid(node)) return 0;

    AVL_GetBalance(node_left(node));
    AVL_GetBalance(node_right(node));

    return AVL_UpdateBalance(node);
}

static void AVL2RBHelper(Node *node, int siblingHeight) {
    // TODO
    if (!node || node == sentinel) {
        return;
    }


    if ((node->height & 1) == 0 || node->height < siblingHeight) {
        node->color = BLACK;
    } else {
        node->color = RED;
    }

    Node *left = node_left(node), *right = node_right(node);

    AVL2RBHelper(left, right->height);
    AVL2RBHelper(right, left->height);
}


void AVL2RB(BT *bt) {
    // TODO
    if (!bt || !IsValid(bt->root)) {
        return;
    }

    AVL_GetHeight(bt->root);
    AVL2RBHelper(bt->root, 0);
}


void AVL_AddFixup(Node *fixupNode, int addedValue, BT *bt) {
    assert(IsValid(fixupNode));

    // see next two children
    Node *child;
    int path = 0;

    if (fixupNode->val > addedValue) {
        child = node_left(fixupNode);
        path--;
    } else {
        child = node_right(fixupNode);
        path++;
    }

    if (child->val > addedValue) {
        path--;
    } else {
        path++;
    }

    if (path == 0) {
        // case 2
        if (fixupNode->balance < 0) {
            AVL_LeftRotate(child, bt);
            path--;
        } else {
            AVL_RightRotate(child, bt);
            path++;
        }
    }

    // case 1
    if (path > 0) {
        AVL_LeftRotate(fixupNode, bt);
    } else if (path < 0) {
        AVL_RightRotate(fixupNode, bt);
    }
}


void AVL_UpdateHeight(Node * node) {
    if (!IsValid(node) || !IsValid(node->par) || node_sibling(node)->height >= node->height) {
        return;
    }

    node->par->height = 1 + node->height;
    AVL_UpdateHeight(node->par);
}

void AVL_Add(int val, BT *bt) {
    assert(bt);

    Node *node = RB_NodeAlloc(0);
    node->height = 1;
    node->val = val;

    if (!IsValid(bt->root)) {
        bt->root = node;
        bt->size++;
        return;
    }

    // find right place
    Node *temp = bt->root;
    Node *parent = sentinel;
    Node *fixupNode = NULL;

    while (IsValid(temp)) {
        parent = temp;

        if (val > temp->val) {
            temp->balance += 1;
            if (temp->balance > 1) {
                if (fixupNode) {
                    fixupNode->balance = fixupNode->balance < -1 ? -1 : 1;
                }
                fixupNode = temp;
            }
            temp = node_right(temp);
        } else {
            temp->balance -= 1;
            if (temp->balance < -1) {
                if (fixupNode) {
                    fixupNode->balance = fixupNode->balance < -1 ? -1 : 1;
                }
                fixupNode = temp;
            }
            temp = node_left(temp);
        }
    }
    assert(IsValid(parent));

    if (parent->val > val) {
        node_left(parent) = node;
    } else {
        node_right(parent) = node;
    }

    node->par = parent;
    bt->size++;

    // update parent height
    AVL_UpdateHeight(node);

    if (fixupNode && abs(fixupNode->balance) > 1) {
        AVL_AddFixup(fixupNode, val, bt);
    }
}


void AVL_DelFixup(Node *node, BT *bt) {
}


void AVL_Del(Node *node, BT *bt) {
}
