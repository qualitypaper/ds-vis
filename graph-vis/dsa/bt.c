#include "bt.h"
#include "base/core.h"
#include "queue.h"

#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

// globals
Node *sentinel = NULL;

// per-tree node arena; grown on overflow, bulk-freed on conversions
#define TREE_ARENA_SIZE KB(64)

static Node *Sentinel_Alloc(void) {
  if (sentinel)
    return sentinel;
#ifdef _WIN32
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  size_t pageSize = si.dwPageSize;
  void *page =
      VirtualAlloc(NULL, pageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  assert(page != NULL);
#else
  long pageSize = sysconf(_SC_PAGESIZE);
  if (pageSize <= 0)
    pageSize = 4096;
  void *page = mmap(NULL, (size_t)pageSize, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(page != MAP_FAILED);
#endif

  Node *s = (Node *)page;
  Node **children = (Node **)((char *)page + sizeof(Node));

  s->id = (ID)s;
  s->children = children;
  s->childCount = 2;
  s->par = s;
  s->val = INT32_MIN;
  s->height = 0;
  s->color = GRAY;

  children[0] = s;
  children[1] = s;

#ifdef _WIN32
  DWORD oldProtect;
  VirtualProtect(page, pageSize, PAGE_READONLY, &oldProtect);
#else
  mprotect(page, (size_t)pageSize, PROT_READ);
#endif

  return s;
}

static int IsValid(const Node *node) {
  if (!node || node == sentinel)
    return 0;
  return 1;
}

// -----------------------------------------------
//                 BT-Trees
// -----------------------------------------------

Node *BT_Build(Arena *arena, int preorder[], int inorder[], int ilower,
               int iupper, int index) {
  if (ilower > iupper)
    return NULL;

  int root = preorder[index];

  int iindex = ilower;
  while (iindex <= iupper && inorder[iindex] != root)
    ++iindex;

  int leftSize = iindex - ilower;

  Node *node = node_alloc(arena, root, 2);
  node->children[0] =
      BT_Build(arena, preorder, inorder, ilower, iindex - 1, index + 1);
  node->children[1] = BT_Build(arena, preorder, inorder, iindex + 1, iupper,
                               index + 1 + leftSize);
  if (node->children[0])
    node->children[0]->par = node;
  if (node->children[1])
    node->children[1]->par = node;
  return node;
}

BT BT_Init(int preorder[], int inorder[], size_t len) {
  assert(len > 0);

  Arena *arena = arena_alloc(TREE_ARENA_SIZE);
  BT bt = {.type = BINARY,
           .arena = arena,
           .root = BT_Build(arena, preorder, inorder, 0, len - 1, 0),
           .size = len};
  return bt;
}

size_t BT_MaxDepth(BT *tree) {
  if (!tree || !tree->root)
    return 0;

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
    if (dep > maxDepth)
      maxDepth = dep;
  }
  return maxDepth;
}

void BT_Add(int val, BT *tree) {
  Node *node = node_alloc(tree->arena, val, 2);
  node->children[0] = tree->root;
  if (tree->root)
    tree->root->par = node;
  tree->root = node;
  tree->size++;
}

void BT_Transplant(Node *root, Node *node, Node *newNode) {
  if (node == root) {
    root = newNode;
    newNode->par = NULL;
    return;
  }

  if (IsLeftChild(node))
    NodeLeft(node->par) = newNode;
  else
    NodeRight(node->par) = newNode;

  if (newNode)
    newNode->par = node->par;
}

void BT_Del(Node *node, BT *tree) {
  if (!node || !tree->root)
    return;

  Node *right = tree->root;
  while (right->children[1])
    right = right->children[1];

  BT_Transplant(tree->root, right, right->children[0]);

  if (right == node) {
    return;
  }

  right->children[0] = node->children[0];
  right->children[1] = node->children[1];
  if (node->children[0])
    node->children[0]->par = right;
  if (node->children[1])
    node->children[1]->par = right;

  BT_Transplant(tree->root, node, right);
  tree->size--;
  // ponytail: removed nodes are abandoned in the arena, never reclaimed until
  // the next full rebuild (conversions) or process exit.
}

void BT_DelByVal(int val, BT *tree) {
  NList queue = NList_Init(tree->size / 2 + 1);
  NList_Add(tree->root, &queue);

  while (queue.len > 0) {
    Node *curr = queue.arr[--queue.len];
    if (!curr)
      continue;
    if (val == curr->val) {
      BT_Del(curr, tree);
      return;
    }
    NList_Add(curr->children[0], &queue);
    NList_Add(curr->children[1], &queue);
  }
}

// -----------------------------------------------
//                  BST-Trees
// -----------------------------------------------

Node *BST_Build(Arena *arena, int preorder[], int index, int upper) {
  if (index > upper)
    return NULL;

  int root = preorder[index];

  Node *node = node_alloc(arena, root, 2);

  int rightChildIndex = index + 1;
  while (rightChildIndex <= upper && preorder[rightChildIndex] <= root)
    rightChildIndex++;

  node->children[0] =
      BST_Build(arena, preorder, index + 1, rightChildIndex - 1);
  node->children[1] = BST_Build(arena, preorder, rightChildIndex, upper);
  if (node->children[0])
    node->children[0]->par = node;
  if (node->children[1])
    node->children[1]->par = node;
  return node;
}

BT BST_Init(int preorder[], size_t len) {
  Arena *arena = arena_alloc(TREE_ARENA_SIZE);
  BT tree = {.type = BST,
             .root = BST_Build(arena, preorder, 0, len - 1),
             .size = len,
             .arena = arena};
  return tree;
}

Node *BST_Search(int val, BT *bt) {
  Node *temp = bt->root;
  while (temp) {
    if (temp->val > val)
      temp = temp->children[0];
    else if (temp->val < val)
      temp = temp->children[1];
    else
      return temp;
  }
  return NULL;
}

void BST_Add(int val, BT *bt) {
  Node *temp = bt->root;
  Node *par = NULL;

  while (temp) {
    par = temp;
    if (temp->val >= val && temp->children[0])
      temp = temp->children[0];
    else if (temp->val < val && temp->children[1])
      temp = temp->children[1];
    else
      break;
  }

  Node *node = node_alloc(bt->arena, val, 2);
  node->par = par;

  if (!par) {
    bt->root = node;
    return;
  }
  if (par->val >= val)
    par->children[0] = node;
  else
    par->children[1] = node;
  bt->size++;
}

void BST_Del(Node *node, BT *bt) {
  if (!node)
    return;

  if (!node->children[0] && !node->children[1]) {
    if (node == bt->root)
      bt->root = NULL;
    else {
      if (node->par->children[0] == node)
        node->par->children[0] = NULL;
      else
        node->par->children[1] = NULL;
    }
    return;
  }
  if (!node->children[0] || !node->children[1]) {
    Node *child = node->children[0] ? node->children[0] : node->children[1];
    BT_Transplant(bt->root, node, child);
    return;
  }

  // find in-order successor (leftmost in right subtree)
  Node *succ = node->children[1];
  while (succ->children[0])
    succ = succ->children[0];

  if (succ->par != node) {
    BT_Transplant(bt->root, succ, succ->children[1]);
    succ->children[1] = node->children[1];
    succ->children[1]->par = succ;
  }
  BT_Transplant(bt->root, node, succ);
  succ->children[0] = node->children[0];
  succ->children[0]->par = succ;

  bt->size--;
}

// -----------------------------------------------
//                  RB-Trees
// -----------------------------------------------

Node *RB_NodeAlloc(Arena *treeArena, int val) {
  if (!sentinel)
    sentinel = Sentinel_Alloc();
  Node *node = node_alloc(treeArena, val, 2);
  node->color = RED;
  node->par = sentinel;
  node->children[0] = sentinel;
  node->children[1] = sentinel;

  return node;
}

BT RB_Init(int preorder[], size_t len) {
  assert(len != 0);

  if (!sentinel)
    sentinel = Sentinel_Alloc();

  BT bt = (BT){.type = RED_BLACK,
               .arena = arena_alloc(TREE_ARENA_SIZE),
               .root = sentinel,
               .size = 0};

  for (size_t i = 0; i < len; i++) {
    RB_Add(preorder[i], &bt);
  }

  return bt;
}

void RB_LeftRotate(Node *node, BT *bt) {
  if (!node || node == sentinel)
    return;
  Node *right = NodeRight(node);
  if (!right || right == sentinel)
    return;

  NodeRight(node) = NodeLeft(right);

  if (NodeRight(node) != sentinel) {
    NodeRight(node)->par = node;
  }

  right->par = node->par;

  if (node->par == sentinel) {
    bt->root = right;
  } else if (node->par->children[0] == node) {
    node->par->children[0] = right;
  } else {
    node->par->children[1] = right;
  }

  NodeLeft(right) = node;
  node->par = right;
}

void RB_RightRotate(Node *node, BT *bt) {
  if (!node || node == sentinel)
    return;
  Node *left = NodeLeft(node);
  if (!left || left == sentinel)
    return;

  NodeLeft(node) = NodeRight(left);

  if (NodeLeft(node) != sentinel) {
    NodeLeft(node)->par = node;
  }

  left->par = node->par;

  if (node->par == sentinel) {
    bt->root = left;
  } else if (node->par->children[0] == node) {
    node->par->children[0] = left;
  } else {
    node->par->children[1] = left;
  }

  NodeRight(left) = node;
  node->par = left;
}

void RB_AddFixup(Node *node, BT *bt) {
  if (node == sentinel)
    return;

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
  Node *node = RB_NodeAlloc(bt->arena, val);

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
  if (!fixupNode)
    return;

  while (fixupNode != bt->root && fixupNode->color == BLACK) {
    Node *p = (fixupNode != sentinel) ? fixupNode->par : parent;
    if (!p || p == sentinel)
      break;

    Node *temp;

    if (fixupNode == NodeLeft(p)) {
      temp = NodeRight(p);

      if (temp->color == RED) {
        // case 1
        temp->color = BLACK;
        p->color = RED;
        RB_LeftRotate(p, bt);
        p = (fixupNode != sentinel) ? fixupNode->par : parent;
        temp = NodeRight(p);
      }

      if (NodeLeft(temp)->color == BLACK && NodeRight(temp)->color == BLACK) {
        // case 2
        temp->color = RED;
        fixupNode = p;
        parent = p->par;
      } else {
        if (NodeRight(temp)->color == BLACK) {
          // case 3: near (left) nephew red, far (right) nephew black
          if (NodeLeft(temp) != sentinel)
            NodeLeft(temp)->color = BLACK;
          temp->color = RED;
          RB_RightRotate(temp, bt);
          temp = NodeRight(p); // sibling changed; re-fetch before case 4
        }

        // case 4
        temp->color = p->color;
        p->color = BLACK;
        if (NodeRight(temp) != sentinel)
          NodeRight(temp)->color = BLACK;
        RB_LeftRotate(p, bt);
        fixupNode = bt->root; // fully resolved; end the loop
      }
    } else {
      // just swap left with right
      temp = NodeLeft(p);

      if (temp->color == RED) {
        // case 1
        temp->color = BLACK;
        p->color = RED;
        RB_RightRotate(p, bt);
        p = (fixupNode != sentinel) ? fixupNode->par : parent;
        temp = NodeLeft(p);
      }

      if (NodeLeft(temp)->color == BLACK && NodeRight(temp)->color == BLACK) {
        // case 2
        temp->color = RED;
        fixupNode = p;
        parent = p->par;
      } else {
        if (NodeLeft(temp)->color == BLACK) {
          // case 3: near (right) nephew red, far (left) nephew black
          if (NodeRight(temp) != sentinel)
            NodeRight(temp)->color = BLACK;
          temp->color = RED;
          RB_LeftRotate(temp, bt);
          temp = NodeLeft(p); // sibling changed; re-fetch before case 4
        }

        // case 4
        temp->color = p->color;
        p->color = BLACK;
        if (NodeLeft(temp) != sentinel)
          NodeLeft(temp)->color = BLACK;
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
  if (!node || node == sentinel)
    return;

  bt->size--;
  COLOR originalColor = node->color;
  Node *fixupNode = sentinel;
  Node *parent = sentinel;

  if (NodeLeft(node) == sentinel) {
    fixupNode = NodeRight(node);
    parent = node->par;
    RB_Transplant(node, fixupNode, bt);
  } else if (NodeRight(node) == sentinel) {
    fixupNode = NodeLeft(node);
    parent = node->par;
    RB_Transplant(node, fixupNode, bt);
  } else {
    Node *temp = NodeRight(node);
    if (temp == sentinel) {
      return;
    }

    while (NodeLeft(temp) != sentinel) {
      temp = NodeLeft(temp);
    }

    originalColor = temp->color;
    fixupNode = NodeRight(temp);

    if (temp->par == node) {
      parent = temp;
      if (fixupNode != sentinel) {
        fixupNode->par = temp;
      }
    } else {
      parent = temp->par;
      RB_Transplant(temp, fixupNode, bt);
      NodeRight(temp) = NodeRight(node);
      if (NodeRight(temp) != sentinel) {
        NodeRight(temp)->par = temp;
      }
    }

    RB_Transplant(node, temp, bt);
    NodeLeft(temp) = NodeLeft(node);
    if (NodeLeft(temp) != sentinel)
      NodeLeft(temp)->par = temp;
    temp->color = node->color;
  }

  if (originalColor == BLACK) {
    RB_DelFixup(fixupNode, parent, bt);
  }
  // ponytail: deleted node stays allocated in the tree arena until the next
  // rebuild
}
/*
 * @returns height from @param root
 */
static int IsAvlConvertable(Node *root) {
  if (!root || root == sentinel)
    return 0;

  Node *l = NodeLeft(root);
  Node *r = NodeRight(root);

  int left = IsAvlConvertable(l);
  if (left == -1) {
    return -1;
  }
  if (l && l != sentinel)
    l->height = left;

  int right = IsAvlConvertable(r);
  if (right == -1) {
    return -1;
  }
  if (r && r != sentinel)
    r->height = right;

  if (abs(right - left) > 1) {
    return -1;
  }
  root->balance = right - left;

  const int height = Max(left, right) + 1;
  root->height = height;

  return height;
}

void RB2AVL(BT *rb) {
  assert(rb);

  if (IsAvlConvertable(rb->root) != -1) {
    rb->type = AVL;
  }
}

// -----------------------------------------------------------
//						AVL-Trees
// -----------------------------------------------------------

void AVL_LeftRotate(Node *node, BT *bt) {
  RB_LeftRotate(node, bt);

  Node *right = node->par;

  node->height = Max(NodeLeft(node)->height, NodeRight(node)->height) + 1;
  right->height = Max(NodeRight(right)->height, node->height + 1);

  node->balance = NodeRight(node)->height - NodeLeft(node)->height;
  right->balance = NodeRight(right)->height - NodeLeft(right)->height;
}

void AVL_RightRotate(Node *node, BT *bt) {
  RB_RightRotate(node, bt);
  Node *left = node->par;

  node->height = Max(NodeLeft(node)->height, NodeRight(node)->height) + 1;
  left->height = Max(NodeLeft(left)->height, node->height + 1);

  node->balance = NodeRight(node)->height - NodeLeft(node)->height;
  left->balance = NodeRight(left)->height - NodeLeft(left)->height;
}

static Node *AVL_UpdateNodeBalance(Node *start, const Node *end) {
  if (!IsValid(start))
    return NULL;

  Node *curr = start;
  Node *fixupNode = NULL;

  do {
    curr->balance = NodeRight(curr)->height - NodeLeft(curr)->height;
    if (abs(curr->balance) > 1) {
      fixupNode = curr;
    }
    curr = curr->par;
  } while (IsValid(curr) && curr != end->par);

  return fixupNode;
}

BT AVL_Init(int preorder[], const size_t len) {
  assert(len > 0);

  BT bt = (BT){.type = AVL,
               .arena = arena_alloc(TREE_ARENA_SIZE),
               .root = NULL,
               .size = 0};

  if (!sentinel)
    sentinel = Sentinel_Alloc();

  for (size_t i = 0; i < len; i++) {
    AVL_Add(preorder[i], &bt);
  }

  return bt;
}

// ponytail: the AVL -> RB / BST -> RB / BST -> AVL conversions rebuild the tree
// by re-inserting the in-order values, which reallocates nodes (so Node.id /
// uiStateMap entries reset). An in-place recolor is not always possible for
// arbitrary shapes, so rebuilding guarantees a valid result regardless of the
// source shape.

static size_t CountTreeNodes(Node *n) {
  if (!n || n == sentinel)
    return 0;
  return 1 + CountTreeNodes(n->children[0]) + CountTreeNodes(n->children[1]);
}

static void CollectTreeInOrder(Node *n, int *vals, size_t *idx) {
  if (!n || n == sentinel)
    return;
  CollectTreeInOrder(n->children[0], vals, idx);
  vals[(*idx)++] = n->val;
  CollectTreeInOrder(n->children[1], vals, idx);
}

void AVL2RB(BT *bt) {
  if (!bt || !IsValid(bt->root)) {
    return;
  }

  const size_t count = CountTreeNodes(bt->root);
  int *vals = malloc(count * sizeof(int));
  size_t idx = 0;
  CollectTreeInOrder(bt->root, vals, &idx);
  arena_reset(
      bt->arena); // ponytail: bulk-free instead of per-node free; chained
                  // overflow arenas are orphaned (leaked) by the reset.

  if (!sentinel)
    sentinel = Sentinel_Alloc();

  bt->root = sentinel;
  bt->size = 0;
  bt->type = RED_BLACK;
  for (size_t i = 0; i < idx; i++) {
    RB_Add(vals[i], bt);
  }
  free(vals);
}

void BST2RB(BT *bt) {
  if (!bt || !bt->root) {
    return;
  }

  const size_t count = CountTreeNodes(bt->root);
  int *vals = malloc(count * sizeof(int));
  size_t idx = 0;
  CollectTreeInOrder(bt->root, vals, &idx);
  arena_reset(bt->arena);

  if (!sentinel)
    sentinel = Sentinel_Alloc();

  bt->root = sentinel;
  bt->size = 0;
  bt->type = RED_BLACK;
  for (size_t i = 0; i < idx; i++) {
    RB_Add(vals[i], bt);
  }
  free(vals);
}

void BST2AVL(BT *bt) {
  if (!bt || !bt->root) {
    return;
  }

  const size_t count = CountTreeNodes(bt->root);
  int *vals = malloc(count * sizeof(int));
  size_t idx = 0;
  CollectTreeInOrder(bt->root, vals, &idx);
  arena_reset(bt->arena);

  if (!sentinel)
    sentinel = Sentinel_Alloc();

  bt->root = sentinel;
  bt->size = 0;
  bt->type = AVL;
  for (size_t i = 0; i < idx; i++) {
    AVL_Add(vals[i], bt);
  }
  free(vals);
}

// RB and AVL both use the shared sentinel for leaves / parent-of-root; a BST
// expects NULL there. Since RB/AVL are already valid BST shapes, converting is
// just replacing sentinel links with NULL in place (node identity is
// preserved).
static void ConvertSentinelToNull(BT *bt) {
  bt->root->par = NULL;

  NQueue queue = NQueue_Init(bt->size > 0 ? bt->size : 4);
  NQueue_Enqueue(bt->root, &queue);

  while (!NQueue_IsEmpty(&queue)) {
    Node *n = NQueue_Dequeue(&queue);
    for (size_t i = 0; i < n->childCount; i++) {
      if (n->children[i] == sentinel) {
        n->children[i] = NULL;
      } else {
        NQueue_Enqueue(n->children[i], &queue);
      }
    }
  }
}

void RB2BST(BT *bt) {
  if (!bt || !IsValid(bt->root)) {
    return;
  }

  ConvertSentinelToNull(bt);
  bt->type = BST;
}

void AVL2BST(BT *bt) {
  if (!bt || !IsValid(bt->root)) {
    return;
  }

  ConvertSentinelToNull(bt);
  bt->type = BST;
}

void AVL_AddFixup(Node *fixupNode, Node *addedNode, BT *bt) {
  assert(IsValid(fixupNode));

  // see next two children
  Node *child;
  int path = 0;

  if (fixupNode->val > addedNode->val) {
    child = NodeLeft(fixupNode);
    path--;
  } else {
    child = NodeRight(fixupNode);
    path++;
  }

  if (child->val > addedNode->val) {
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

  AVL_UpdateHeight(addedNode);
  AVL_UpdateNodeBalance(addedNode, sentinel);
}

Node *AVL_UpdateHeight(Node *node) {
  if (!IsValid(node))
    return node;

  Node *curr = node;

  while (IsValid(curr->par) && NodeSibling(curr)->height < curr->height) {
    curr->par->height = 1 + curr->height;
    curr = curr->par;
  }

  while (IsValid(curr->par) && NodeSibling(curr)->height == curr->height &&
         curr->par->balance != 0) {
    curr->par->height = 1 + curr->height;
    curr = curr->par;
  }

  return curr;
}

static Node *AVL_FindInsertNode(const int insertVal, Node *root) {
  Node *temp = root;
  Node *parent = sentinel;

  while (IsValid(temp)) {
    assert(abs(temp->balance) <= 1 && "Balance of the node must be -1, 0 or 1");
    parent = temp;

    if (insertVal > temp->val) {
      temp = NodeRight(temp);
    } else {
      temp = NodeLeft(temp);
    }
  }
  assert(IsValid(parent));

  return parent;
}

void AVL_Add(const int val, BT *bt) {
  assert(bt);

  Node *node = RB_NodeAlloc(bt->arena, 0);
  node->height = 1;
  node->val = val;

  if (!IsValid(bt->root)) {
    bt->root = node;
    bt->size++;
    return;
  }

  // find right place
  Node *parent = AVL_FindInsertNode(val, bt->root);

  if (parent->val > val) {
    NodeLeft(parent) = node;
  } else {
    NodeRight(parent) = node;
  }

  node->par = parent;
  bt->size++;

  // update parent height
  const Node *lastUpdatedNode = AVL_UpdateHeight(node);
  Node *fixupNode = AVL_UpdateNodeBalance(parent, lastUpdatedNode);

  if (fixupNode) {
    AVL_AddFixup(fixupNode, node, bt);
  }
}

void AVL_DelFixup(Node *node, BT *bt) {}

void AVL_Del(Node *node, BT *bt) {}
