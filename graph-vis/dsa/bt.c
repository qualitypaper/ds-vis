#include "bt.h"
#include "base/core.h"
#include "queue.h"

#include <assert.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

// per-tree node arena; grown on overflow, bulk-freed on conversions
#define TREE_ARENA_SIZE KB(64)

static BNode* Sentinel_Alloc(void)
{
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t pageSize = si.dwPageSize;
    void* page =
        VirtualAlloc(NULL, pageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    assert(page != NULL);
#else
    long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize <= 0)
        pageSize = 4096;
    void* page = mmap(NULL, (size_t)pageSize, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(page != MAP_FAILED);
#endif

    BNode* s = (BNode*)page;

    s->id = (ID)s;
    s->left = s;
    s->right = s;
    s->par = s;
    s->val = INT32_MIN;
    s->height = 0;
    s->balance = 0;
    s->color = GRAY;

#ifdef _WIN32
    DWORD oldProtect;
    VirtualProtect(page, pageSize, PAGE_READONLY, &oldProtect);
#else
    mprotect(page, (size_t)pageSize, PROT_READ);
#endif

    return s;
}

static BNode* FindInsertNode(const int insertVal, BNode* root)
{
    BNode* temp = root;
    BNode* parent = NULL;

    while (IsValid(temp))
    {
        assert(abs(temp->balance) <= 1 && "Balance of the node must be -1, 0 or 1");
        parent = temp;

        if (insertVal > temp->val)
        {
            temp = BRight(temp);
        }
        else
        {
            temp = BLeft(temp);
        }
    }
    assert(IsValid(parent));

    return parent;
}

// -----------------------------------------------
//                 BT-Trees
// -----------------------------------------------

BNode* BT_Build(Arena* arena, int preorder[], int inorder[], int ilower,
               int iupper, int index)
{
    if (ilower > iupper)
        return NULL;

    int root = preorder[index];

    int iindex = ilower;
    while (iindex <= iupper && inorder[iindex] != root)
        ++iindex;

    int leftSize = iindex - ilower;

    BNode* node = bnode_alloc(arena, root);
    node->left =
        BT_Build(arena, preorder, inorder, ilower, iindex - 1, index + 1);
    node->right = BT_Build(arena, preorder, inorder, iindex + 1, iupper,
                           index + 1 + leftSize);
    if (node->left)
        node->left->par = node;
    if (node->right)
        node->right->par = node;
    return node;
}

BT BT_Init(const S32Array preorder, const S32Array inorder)
{
    assert(preorder.count > 0);

    Arena* arena = arena_alloc(TREE_ARENA_SIZE);
    BT bt = {
        .type = BINARY,
        .arena = arena,
        .sentinel = NULL,
        .root = BT_Build(arena, preorder.v, inorder.v, 0, preorder.count, 0),
        .size = preorder.count
    };
    return bt;
}

size_t BT_MaxDepth(BT* tree)
{
    if (!tree || !tree->root)
        return 0;

    size_t maxDepth = 0;
    struct AQueue depth = AQ_Init(tree->arena, tree->size);
    NQueue queue = NQueue_Init(tree->arena, tree->size / 2);
    NQueue_Enqueue(tree->root, &queue);
    AQ_Enqueue(1, &depth);

    while (!NQueue_IsEmpty(&queue))
    {
        int dep = AQ_Dequeue(&depth);
        BNode* node = NQueue_Dequeue(&queue);

        if (node->left)
        {
            NQueue_Enqueue(node->left, &queue);
            AQ_Enqueue(dep + 1, &depth);
        }
        if (node->right)
        {
            NQueue_Enqueue(node->right, &queue);
            AQ_Enqueue(dep + 1, &depth);
        }
        if (dep > maxDepth)
            maxDepth = dep;
    }
    return maxDepth;
}

void BT_Add(int val, BT* tree)
{
    BNode* node = bnode_alloc(tree->arena, val);
    node->left = tree->root;
    if (tree->root)
        tree->root->par = node;
    tree->root = node;
    tree->size++;
}

void BT_Transplant(BNode* root, BNode* node, BNode* newNode)
{
    if (node == root)
    {
        root = newNode;
        newNode->par = NULL;
        return;
    }

    if (BIsLeftChild(node))
        BLeft(node->par) = newNode;
    else
        BRight(node->par) = newNode;

    if (newNode)
        newNode->par = node->par;
}

void BT_Del(BNode* node, BT* tree)
{
    if (!node || !tree->root)
        return;

    BNode* right = tree->root;
    while (right->right)
        right = right->right;

    BT_Transplant(tree->root, right, right->left);

    if (right == node)
    {
        return;
    }

    right->left = node->left;
    right->right = node->right;
    if (node->left)
        node->left->par = right;
    if (node->right)
        node->right->par = right;

    BT_Transplant(tree->root, node, right);
    tree->size--;
    // ponytail: removed nodes are abandoned in the arena, never reclaimed until
    // the next full rebuild (conversions) or process exit.
}

void BT_DelByVal(int val, BT* tree)
{
    NList queue = NList_Init(tree->arena, tree->size / 2 + 1);
    NList_Add(tree->root, &queue);

    while (queue.len > 0)
    {
        BNode* curr = queue.arr[--queue.len];
        if (!curr)
            continue;
        if (val == curr->val)
        {
            BT_Del(curr, tree);
            return;
        }
        NList_Add(curr->left, &queue);
        NList_Add(curr->right, &queue);
    }
}

// -----------------------------------------------
//                  BST-Trees
// -----------------------------------------------

BNode* BST_Build(Arena* arena, int preorder[], int index, int upper)
{
    if (index > upper)
        return NULL;

    int root = preorder[index];

    BNode* node = bnode_alloc(arena, root);

    int rightChildIndex = index + 1;
    while (rightChildIndex <= upper && preorder[rightChildIndex] <= root)
        rightChildIndex++;

    node->left =
        BST_Build(arena, preorder, index + 1, rightChildIndex - 1);
    node->right = BST_Build(arena, preorder, rightChildIndex, upper);
    if (node->left)
        node->left->par = node;
    if (node->right)
        node->right->par = node;
    return node;
}

BT BST_Init(int preorder[], size_t len)
{
    Arena* arena = arena_alloc(TREE_ARENA_SIZE);
    BT tree = {
        .type = BST,
        .root = BST_Build(arena, preorder, 0, len - 1),
        .size = len,
        .arena = arena
    };
    return tree;
}

BNode* BST_Search(int val, BT* bt)
{
    BNode* temp = bt->root;
    while (temp)
    {
        if (temp->val > val)
            temp = temp->left;
        else if (temp->val < val)
            temp = temp->right;
        else
            return temp;
    }
    return NULL;
}

void BST_Add(int val, BT* bt)
{
    BNode* temp = bt->root;
    BNode* par = NULL;

    while (temp)
    {
        par = temp;
        if (temp->val >= val && temp->left)
            temp = temp->left;
        else if (temp->val < val && temp->right)
            temp = temp->right;
        else
            break;
    }

    BNode* node = bnode_alloc(bt->arena, val);
    node->par = par;

    if (!par)
    {
        bt->root = node;
        return;
    }
    if (par->val >= val)
        par->left = node;
    else
        par->right = node;
    bt->size++;
}

void BST_Del(BNode* node, BT* bt)
{
    if (!node)
        return;

    if (!node->left && !node->right)
    {
        if (node == bt->root)
            bt->root = NULL;
        else
        {
            if (node->par->left == node)
                node->par->left = NULL;
            else
                node->par->right = NULL;
        }
        return;
    }
    if (!node->left || !node->right)
    {
        BNode* child = node->left ? node->left : node->right;
        BT_Transplant(bt->root, node, child);
        return;
    }

    // find in-order successor (leftmost in right subtree)
    BNode* succ = node->right;
    while (succ->left)
        succ = succ->left;

    if (succ->par != node)
    {
        BT_Transplant(bt->root, succ, succ->right);
        succ->right = node->right;
        succ->right->par = succ;
    }
    BT_Transplant(bt->root, node, succ);
    succ->left = node->left;
    succ->left->par = succ;

    bt->size--;
}

// -----------------------------------------------
//                  RB-Trees
// -----------------------------------------------

BNode* RB_BNodeAlloc(Arena* treeArena, int val, BNode* sentinel)
{
    BNode* node = bnode_alloc_nil(treeArena, val, sentinel);
    node->color = RED;

    return node;
}

BT RB_Init(int preorder[], size_t len)
{
    assert(len != 0);

    BNode* sentinel = Sentinel_Alloc();

    BT bt = (BT){
        .type = RED_BLACK,
        .arena = arena_alloc(TREE_ARENA_SIZE),
        .sentinel = sentinel,
        .root = sentinel,
        .size = 0
    };

    for (size_t i = 0; i < len; i++)
    {
        RB_Add(preorder[i], &bt);
    }

    return bt;
}

void RB_LeftRotate(BNode* node, BT* bt)
{
    if (!IsValid(node))
        return;
    BNode* right = BRight(node);
    if (!IsValid(right))
        return;

    BRight(node) = BLeft(right);

    if (IsValid(BRight(node)))
    {
        BRight(node)->par = node;
    }

    right->par = node->par;

    if (!IsValid(node->par))
    {
        bt->root = right;
    }
    else if (node->par->left == node)
    {
        node->par->left = right;
    }
    else
    {
        node->par->right = right;
    }

    BLeft(right) = node;
    node->par = right;
}

void RB_RightRotate(BNode* node, BT* bt)
{
    if (!IsValid(node))
        return;
    BNode* left = BLeft(node);
    if (!IsValid(left))
        return;

    BLeft(node) = BRight(left);

    if (IsValid(BLeft(node)))
    {
        BLeft(node)->par = node;
    }

    left->par = node->par;

    if (!IsValid(node->par))
    {
        bt->root = left;
    }
    else if (BIsLeftChild(node))
    {
        BLeft(node->par) = left;
    }
    else
    {
        BRight(node->par) = left;
    }

    BRight(left) = node;
    node->par = left;
}

void RB_AddFixup(BNode* node, BT* bt)
{
    if (!IsValid(node))
        return;

    while (node->par->color == RED && IsValid(node->par))
    {
        BNode* parent = node->par;

        if (BIsLeftChild(parent))
        {
            BNode* uncle = BRight(parent->par);

            if (uncle->color == RED)
            {
                uncle->color = GRAY;
                parent->color = GRAY;
                parent->par->color = RED;
                node = parent->par;
            }
            else
            {
                if (BIsRightChild(node))
                {
                    node = parent;
                    RB_LeftRotate(node, bt);
                }
                node->par->color = GRAY;
                node->par->par->color = RED;

                RB_RightRotate(node->par->par, bt);
            }
        }
        else
        {
            BNode* uncle = BLeft(parent->par);

            if (uncle->color == RED)
            {
                uncle->color = GRAY;
                parent->color = GRAY;
                parent->par->color = RED;
                node = parent->par;
            }
            else
            {
                if (BIsLeftChild(node))
                {
                    node = parent;
                    RB_RightRotate(node, bt);
                }
                node->par->color = GRAY;
                node->par->par->color = RED;

                RB_LeftRotate(node->par->par, bt);
            }
        }
    }

    if (IsValid(bt->root))
    {
        bt->root->color = GRAY;
    }
}

void RB_Add(int val, BT* bt)
{
    BNode* node = RB_BNodeAlloc(bt->arena, val, bt->sentinel);

    BNode* dest = bt->sentinel;
    BNode* temp = bt->root;

    while (IsValid(temp))
    {
        dest = temp;

        if (temp->val < val)
        {
            temp = temp->right;
        }
        else
        {
            temp = temp->left;
        }
    }

    node->par = dest;

    if (!IsValid(dest))
    {
        bt->root = node;
    }
    else if (dest->val < val)
    {
        // new right child
        dest->right = node;
    }
    else
    {
        // new left child
        dest->left = node;
    }

    node->color = RED;
    RB_AddFixup(node, bt);
    bt->size++;
}

void RB_Transplant(BNode* oldNode, BNode* newNode, BT* bt)
{
    if (!IsValid(oldNode->par))
    {
        bt->root = newNode;
    }
    else if (oldNode == oldNode->par->left)
    {
        oldNode->par->left = newNode;
    }
    else
    {
        oldNode->par->right = newNode;
    }
    if (IsValid(newNode))
    {
        newNode->par = oldNode->par;
    }
}

void RB_DelFixup(BNode* fixupNode, BNode* parent, BT* bt)
{
    if (!fixupNode)
        return;

    while (fixupNode != bt->root && fixupNode->color == BLACK)
    {
        BNode* p = IsValid(fixupNode) ? fixupNode->par : parent;
        if (!p || !IsValid(p))
            break;

        BNode* temp;

        if (fixupNode == BLeft(p))
        {
            temp = BRight(p);

            if (temp->color == RED)
            {
                // case 1
                temp->color = BLACK;
                p->color = RED;
                RB_LeftRotate(p, bt);
                p = IsValid(fixupNode) ? fixupNode->par : parent;
                temp = BRight(p);
            }

            if (BLeft(temp)->color == BLACK && BRight(temp)->color == BLACK)
            {
                // case 2
                temp->color = RED;
                fixupNode = p;
                parent = p->par;
            }
            else
            {
                if (BRight(temp)->color == BLACK)
                {
                    // case 3: near (left) nephew red, far (right) nephew black
                    if (IsValid(BLeft(temp)))
                        BLeft(temp)->color = BLACK;
                    temp->color = RED;
                    RB_RightRotate(temp, bt);
                    temp = BRight(p); // sibling changed; re-fetch before case 4
                }

                // case 4
                temp->color = p->color;
                p->color = BLACK;
                if (IsValid(BRight(temp)))
                    BRight(temp)->color = BLACK;
                RB_LeftRotate(p, bt);
                fixupNode = bt->root; // fully resolved; end the loop
            }
        }
        else
        {
            // just swap left with right
            temp = BLeft(p);

            if (temp->color == RED)
            {
                // case 1
                temp->color = BLACK;
                p->color = RED;
                RB_RightRotate(p, bt);
                p = IsValid(fixupNode) ? fixupNode->par : parent;
                temp = BLeft(p);
            }

            if (BLeft(temp)->color == BLACK && BRight(temp)->color == BLACK)
            {
                // case 2
                temp->color = RED;
                fixupNode = p;
                parent = p->par;
            }
            else
            {
                if (BLeft(temp)->color == BLACK)
                {
                    // case 3: near (right) nephew red, far (left) nephew black
                    if (IsValid(BRight(temp)))
                        BRight(temp)->color = BLACK;
                    temp->color = RED;
                    RB_LeftRotate(temp, bt);
                    temp = BLeft(p); // sibling changed; re-fetch before case 4
                }

                // case 4
                temp->color = p->color;
                p->color = BLACK;
                if (IsValid(BLeft(temp)))
                    BLeft(temp)->color = BLACK;
                RB_RightRotate(p, bt);
                fixupNode = bt->root; // fully resolved; end the loop
            }
        }
    }

    if (IsValid(fixupNode))
    {
        fixupNode->color = BLACK;
    }
}

void RB_Del(BNode* node, BT* bt)
{
    if (!IsValid(node))
        return;

    bt->size--;
    COLOR originalColor = node->color;
    BNode* fixupNode = bt->sentinel;
    BNode* parent = bt->sentinel;

    if (!IsValid(BLeft(node)))
    {
        fixupNode = BRight(node);
        parent = node->par;
        RB_Transplant(node, fixupNode, bt);
    }
    else if (!IsValid(BRight(node)))
    {
        fixupNode = BLeft(node);
        parent = node->par;
        RB_Transplant(node, fixupNode, bt);
    }
    else
    {
        BNode* temp = BRight(node);
        if (!IsValid(temp))
        {
            return;
        }

        while (IsValid(BLeft(temp)))
        {
            temp = BLeft(temp);
        }

        originalColor = temp->color;
        fixupNode = BRight(temp);

        if (temp->par == node)
        {
            parent = temp;
            if (IsValid(fixupNode))
            {
                fixupNode->par = temp;
            }
        }
        else
        {
            parent = temp->par;
            RB_Transplant(temp, fixupNode, bt);
            BRight(temp) = BRight(node);
            if (IsValid(BRight(temp)))
            {
                BRight(temp)->par = temp;
            }
        }

        RB_Transplant(node, temp, bt);
        BLeft(temp) = BLeft(node);
        if (IsValid(BLeft(temp)))
            BLeft(temp)->par = temp;
        temp->color = node->color;
    }

    if (originalColor == BLACK)
    {
        RB_DelFixup(fixupNode, parent, bt);
    }
    // ponytail: deleted node stays allocated in the tree arena until the next
    // rebuild
}

/*
 * @returns height from @param root
 */
static int IsAvlConvertable(BNode* root)
{
    if (!IsValid(root))
        return 0;

    BNode* l = BLeft(root);
    BNode* r = BRight(root);

    int left = IsAvlConvertable(l);
    if (left == -1)
    {
        return -1;
    }
    if (IsValid(l))
        l->height = left;

    int right = IsAvlConvertable(r);
    if (right == -1)
    {
        return -1;
    }
    if (IsValid(r))
        r->height = right;

    if (abs(right - left) > 1)
    {
        return -1;
    }
    root->balance = right - left;

    const int height = Max(left, right) + 1;
    root->height = height;

    return height;
}

void RB2AVL(BT* rb)
{
    assert(rb);

    if (IsAvlConvertable(rb->root) != -1)
    {
        // replace colors
        NQueue queue = NQueue_Init(rb->arena, 4);
        NQueue_Enqueue(rb->root, &queue);
        while (!NQueue_IsEmpty(&queue))
        {
            BNode* curr = NQueue_Dequeue(&queue);
            curr->color = GRAY;
            if (IsValid(BLeft(curr)))
                NQueue_Enqueue(BLeft(curr), &queue);

            if (IsValid(BRight(curr)))
                NQueue_Enqueue(BRight(curr), &queue);
        }

        rb->type = AVL;
    }
    else
    {
        fprintf(stderr, "This red black tree isn't convertable into AVL-Tree.\n");
    }
}

// -----------------------------------------------------------
//						AVL-Trees
// -----------------------------------------------------------

static void AVL_UpdateNode(BNode* n)
{
    if (!IsValid(n))
        return;
    n->height = Max(BLeft(n)->height, BRight(n)->height) + 1;
    n->balance = BRight(n)->height - BLeft(n)->height;
}

void AVL_LeftRotate(BNode* node, BT* bt)
{
    BNode* right = BRight(node);
    RB_LeftRotate(node, bt);
    AVL_UpdateNode(node);
    AVL_UpdateNode(right);
}

void AVL_RightRotate(BNode* node, BT* bt)
{
    BNode* left = BLeft(node);
    RB_RightRotate(node, bt);
    AVL_UpdateNode(node);
    AVL_UpdateNode(left);
}

BT AVL_Init(int preorder[], const size_t len)
{
    assert(len > 0);

    BNode* sentinel = Sentinel_Alloc();

    BT bt = (BT){
        .type = AVL,
        .arena = arena_alloc(TREE_ARENA_SIZE),
        .sentinel = sentinel,
        .root = NULL,
        .size = 0
    };

    for (size_t i = 0; i < len; i++)
    {
        AVL_Add(preorder[i], &bt);
    }

    return bt;
}

// ponytail: the AVL -> RB / BST -> RB / BST -> AVL conversions rebuild the tree
// by re-inserting the in-order values, which reallocates nodes (so BNode.id /
// uiStateMap entries reset). An in-place recolor is not always possible for
// arbitrary shapes, so rebuilding guarantees a valid result regardless of the
// source shape.

static size_t CountTreeNodes(BNode* n)
{
    if (!IsValid(n))
        return 0;
    return 1 + CountTreeNodes(n->left) + CountTreeNodes(n->right);
}

static void CollectTreeInOrder(BNode* n, int* vals, size_t* idx)
{
    if (!IsValid(n))
        return;
    CollectTreeInOrder(n->left, vals, idx);
    vals[(*idx)++] = n->val;
    CollectTreeInOrder(n->right, vals, idx);
}

void AVL2RB(BT* bt)
{
    if (!bt || !IsValid(bt->root))
    {
        return;
    }

    const size_t count = CountTreeNodes(bt->root);
    Arena* temp = arena_alloc(count * sizeof(int));
    int* vals = PushArray(temp, int, count);
    size_t idx = 0;
    CollectTreeInOrder(bt->root, vals, &idx);
    arena_reset(bt->arena);

    if (!bt->sentinel)
        bt->sentinel = Sentinel_Alloc();

    bt->root = bt->sentinel;
    bt->size = 0;
    bt->type = RED_BLACK;
    for (size_t i = 0; i < idx; i++)
    {
        RB_Add(vals[i], bt);
    }
    arena_free(temp);
}

void BST2RB(BT* bt)
{
    if (!bt || !bt->root)
    {
        return;
    }

    const size_t count = CountTreeNodes(bt->root);
    Arena* temp = arena_alloc(count * sizeof(int));
    int* vals = PushArray(temp, int, count);
    size_t idx = 0;
    CollectTreeInOrder(bt->root, vals, &idx);
    arena_reset(bt->arena);

    if (!bt->sentinel)
        bt->sentinel = Sentinel_Alloc();

    bt->root = bt->sentinel;
    bt->size = 0;
    bt->type = RED_BLACK;
    for (size_t i = 0; i < idx; i++)
    {
        RB_Add(vals[i], bt);
    }
    arena_free(temp);
}

void BST2AVL(BT* bt)
{
    if (!bt || !bt->root)
    {
        return;
    }

    const size_t count = CountTreeNodes(bt->root);
    Arena* temp = arena_alloc(count * sizeof(int));
    int* vals = PushArray(temp, int, count);
    size_t idx = 0;
    CollectTreeInOrder(bt->root, vals, &idx);
    arena_reset(bt->arena);

    if (!bt->sentinel)
        bt->sentinel = Sentinel_Alloc();

    bt->root = bt->sentinel;
    bt->size = 0;
    bt->type = AVL;
    for (size_t i = 0; i < idx; i++)
    {
        AVL_Add(vals[i], bt);
    }
    arena_free(temp);
}

// RB and AVL both use the shared sentinel for leaves / parent-of-root; a BST
// expects NULL there. Since RB/AVL are already valid BST shapes, converting is
// just replacing sentinel links with NULL in place (node identity is
// preserved).
static void ConvertSentinelToNull(BT* bt)
{
    bt->root->par = NULL;

    NQueue queue = NQueue_Init(bt->arena, bt->size > 0 ? bt->size : 4);
    NQueue_Enqueue(bt->root, &queue);

    while (!NQueue_IsEmpty(&queue))
    {
        BNode* n = NQueue_Dequeue(&queue);

        if (!IsValid(n->left))
            n->left = NULL;
        else
            NQueue_Enqueue(n->left, &queue);

        if (!IsValid(n->right))
            n->right = NULL;
        else
            NQueue_Enqueue(n->right, &queue);
    }
}

void RB2BST(BT* bt)
{
    if (!bt || !IsValid(bt->root))
    {
        return;
    }

    ConvertSentinelToNull(bt);
    bt->type = BST;
}

void AVL2BST(BT* bt)
{
    if (!bt || !IsValid(bt->root))
    {
        return;
    }

    ConvertSentinelToNull(bt);
    bt->type = BST;
}

void AVL_AddFixup(BNode* inserted, BT* bt)
{
    BNode* curr = inserted->par;

    while (IsValid(curr))
    {
        AVL_UpdateNode(curr);

        if (curr->balance > 1)
        {
            // right-heavy: RL double rotation or RR single
            if (BRight(curr)->balance < 0)
                AVL_RightRotate(BRight(curr), bt);
            AVL_LeftRotate(curr, bt);
            break; // rotation restores the subtree to its pre-insertion height
        }
        if (curr->balance < -1)
        {
            // left-heavy: LR double rotation or LL single
            if (BLeft(curr)->balance > 0)
                AVL_LeftRotate(BLeft(curr), bt);
            AVL_RightRotate(curr, bt);
            break;
        }

        curr = curr->par;
    }
}


void AVL_Add(const int val, BT* bt)
{
    assert(bt);

    BNode* node = RB_BNodeAlloc(bt->arena, 0, bt->sentinel);
    node->height = 1;
    node->val = val;

    if (!IsValid(bt->root))
    {
        bt->root = node;
        bt->size++;
        return;
    }

    // find right place
    BNode* parent = FindInsertNode(val, bt->root);

    if (parent->val >= val)
    {
        BLeft(parent) = node;
    }
    else
    {
        BRight(parent) = node;
    }

    node->par = parent;
    bt->size++;

    AVL_AddFixup(node, bt);
}

void AVL_DelFixup(BNode* node, BT* bt)
{
}

void AVL_Del(BNode* node, BT* bt)
{
}
