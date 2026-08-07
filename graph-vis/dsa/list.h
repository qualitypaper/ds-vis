#ifndef GRAPH_VIS_LIST
#define GRAPH_VIS_LIST

#include <stdlib.h>
#include "base/core.h"

#define GROWTH_FACTOR 1.6

typedef struct List List;

struct List
{
    S32* arr;
    U64 len;
    U64 cap;
};

List L_Init(size_t cap);
void L_Grow(List* list);
void L_Add(int val, List* list);
void L_Del(size_t index, List* list);

#define DEFINE_LIST(T, Name)                                                                          \
typedef struct Name { T* arr; size_t len, cap; } Name;                                                             \
static inline struct Name Name##_Init(size_t cap) {                                                   \
    struct Name l = { (T*) malloc(cap * sizeof(T)), 0, cap };                                         \
    return l;                                                                                         \
}                                                                                                     \
static inline void Name##_Grow(struct Name* l) {                                                      \
    l->cap = (size_t)(l->cap * GROWTH_FACTOR);                                                        \
    l->arr = (T*) realloc(l->arr, l->cap * sizeof(T));                                                \
}                                                                                                     \
static inline void Name##_Add(T val, struct Name* l) {                                                \
    if (l->len >= l->cap) Name##_Grow(l);                                                             \
    l->arr[l->len++] = val;                                                                           \
}                                                                                                     \
static inline void Name##_Del(size_t i, struct Name* l) {                                             \
    if (l->len == 0) return;                                                                          \
    MemoryCopy(&l->arr[i], &l->arr[i + 1], (l->len - i - 1) * sizeof(T));                               \
    l->len--;                                                                                         \
}

////////////////////////////////

typedef struct SLinkedNode SLinkedNode;

struct SLinkedNode
{
    SLinkedNode* next;
    S32 val;
};

typedef struct SLinkedList SLinkedList;

struct SLinkedList
{
    SLinkedNode* head;
};

typedef struct LinkedNode LinkedNode;

struct LinkedNode
{
    LinkedNode* next;
    LinkedNode* prev;
    S32 val;
};

typedef struct LinkedList LinkedList;

struct LinkedList
{
    LinkedNode* head;
    LinkedNode* tail;
};

//////////////////////////////
//~ Concrete linked lists (S32, null-terminated)

SLinkedList SLL_Init(void);
void SLL_PushFront(S32 val, SLinkedList* l);
void SLL_PushBack(S32 val, SLinkedList* l);
S32  SLL_PopFront(SLinkedList* l);
SLinkedNode* SLL_Find(S32 val, const SLinkedList* l);
void SLL_Delete(SLinkedNode* node, SLinkedList* l);
size_t SLL_Count(const SLinkedList* l);
void SLL_Free(SLinkedList* l);

LinkedList LL_Init(void);
void LL_PushFront(S32 val, LinkedList* l);
void LL_PushBack(S32 val, LinkedList* l);
S32  LL_PopFront(LinkedList* l);
S32  LL_PopBack(LinkedList* l);
LinkedNode* LL_Find(S32 val, const LinkedList* l);
void LL_Delete(LinkedNode* node, LinkedList* l);
size_t LL_Count(const LinkedList* l);
void LL_Free(LinkedList* l);

//////////////////////////////
//~ Generic linked lists (any T/Node/List).
//
// END is the terminator expression and is what makes the same body work for
// both termination styles:
//   END = 0            -> NULL-terminated list; List must have a `head` field
//                         (and `tail` for the doubly-linked family).
//   END = &l->nil      -> sentinel-terminated list; the sentinel doubles as the
//                         header. Caller wires it once: l.nil.next = l.nil.prev = &l.nil.
// Node must have `val` (+ `next`; + `prev` for doubly). List must have `head`.

#define DEFINE_SINGLY_LINKED_LIST(T, NodeT, ListT, END)                                            \
    static inline void ListT##_PushFront(T val, struct ListT* l) {                                 \
        struct NodeT* n = (struct NodeT*) malloc(sizeof(struct NodeT));                            \
        n->val = val;                                                                              \
        n->next = l->head;                                                                         \
        l->head = n;                                                                               \
    }                                                                                              \
    static inline void ListT##_PushBack(T val, struct ListT* l) {                                  \
        struct NodeT* n = (struct NodeT*) malloc(sizeof(struct NodeT));                            \
        n->val = val;                                                                              \
        n->next = END;                                                                             \
        if (l->head == END) l->head = n;                                                           \
        else { struct NodeT* c = l->head; while (c->next != END) c = c->next; c->next = n; }       \
    }                                                                                              \
    static inline T ListT##_PopFront(struct ListT* l) {                                            \
        struct NodeT* n = l->head;                                                                 \
        T val = n->val;                                                                            \
        l->head = n->next;                                                                         \
        free(n);                                                                                   \
        return val;                                                                                \
    }                                                                                              \
    static inline struct NodeT* ListT##_Find(T val, const struct ListT* l) {                       \
        for (struct NodeT* n = l->head; n != END; n = n->next)                                     \
            if (n->val == val) return n;                                                           \
        return END;                                                                                \
    }                                                                                              \
    static inline void ListT##_Delete(struct NodeT* node, struct ListT* l) {                       \
        if (l->head == node) l->head = node->next;                                                 \
        else { struct NodeT* p = l->head;                                                          \
               while (p != END && p->next != node) p = p->next;                                    \
               if (p != END) p->next = node->next; }                                               \
        free(node);                                                                                \
    }                                                                                              \
    static inline size_t ListT##_Count(const struct ListT* l) {                                    \
        size_t c = 0;                                                                              \
        for (struct NodeT* n = l->head; n != END; n = n->next) c++;                                 \
        return c;                                                                                  \
    }                                                                                              \
    static inline void ListT##_Free(struct ListT* l) {                                             \
        struct NodeT* n = l->head;                                                                 \
        while (n != END) { struct NodeT* nx = n->next; free(n); n = nx; }                          \
        l->head = END;                                                                             \
    }

#define DEFINE_LINKED_LIST(T, NodeT, ListT, END)                                                   \
    static inline struct NodeT* ListT##_First(const struct ListT* l) {                             \
        return END ? END->next : l->head;                                                          \
    }                                                                                              \
    static inline struct NodeT* ListT##_Last(const struct ListT* l) {                              \
        return END ? END->prev : l->tail;                                                          \
    }                                                                                              \
    static inline int ListT##_IsEmpty(const struct ListT* l) {                                     \
        return ListT##_First(l) == END;                                                            \
    }                                                                                              \
    static inline void ListT##_PushFront(T val, struct ListT* l) {                                 \
        struct NodeT* n = (struct NodeT*) malloc(sizeof(struct NodeT));                            \
        n->val = val;                                                                              \
        if (END) {                                                                                 \
            n->prev = END;                                                                         \
            n->next = END->next;                                                                   \
            END->next->prev = n;                                                                   \
            END->next = n;                                                                         \
        } else {                                                                                   \
            n->prev = 0;                                                                           \
            n->next = l->head;                                                                     \
            if (l->head) l->head->prev = n;                                                        \
            else l->tail = n;                                                                      \
            l->head = n;                                                                           \
        }                                                                                          \
    }                                                                                              \
    static inline void ListT##_PushBack(T val, struct ListT* l) {                                  \
        struct NodeT* n = (struct NodeT*) malloc(sizeof(struct NodeT));                            \
        n->val = val;                                                                              \
        if (END) {                                                                                 \
            n->prev = END->prev;                                                                   \
            n->next = END;                                                                         \
            END->prev->next = n;                                                                   \
            END->prev = n;                                                                         \
        } else {                                                                                   \
            n->next = 0;                                                                           \
            n->prev = l->tail;                                                                     \
            if (l->tail) l->tail->next = n;                                                        \
            else l->head = n;                                                                      \
            l->tail = n;                                                                           \
        }                                                                                          \
    }                                                                                              \
    static inline T ListT##_PopFront(struct ListT* l) {                                            \
        struct NodeT* n = ListT##_First(l);                                                        \
        T val = n->val;                                                                            \
        if (END) {                                                                                 \
            END->next = n->next;                                                                   \
            n->next->prev = END;                                                                   \
        } else {                                                                                   \
            l->head = n->next;                                                                     \
            if (l->head) l->head->prev = 0;                                                        \
            else l->tail = 0;                                                                      \
        }                                                                                          \
        free(n);                                                                                   \
        return val;                                                                                \
    }                                                                                              \
    static inline T ListT##_PopBack(struct ListT* l) {                                             \
        struct NodeT* n = ListT##_Last(l);                                                         \
        T val = n->val;                                                                            \
        if (END) {                                                                                 \
            END->prev = n->prev;                                                                   \
            n->prev->next = END;                                                                   \
        } else {                                                                                   \
            l->tail = n->prev;                                                                     \
            if (l->tail) l->tail->next = 0;                                                        \
            else l->head = 0;                                                                      \
        }                                                                                          \
        free(n);                                                                                   \
        return val;                                                                                \
    }                                                                                              \
    static inline struct NodeT* ListT##_Find(T val, const struct ListT* l) {                       \
        for (struct NodeT* n = ListT##_First(l); n != END; n = n->next)                            \
            if (n->val == val) return n;                                                           \
        return END;                                                                                \
    }                                                                                              \
    static inline void ListT##_Delete(struct NodeT* node, struct ListT* l) {                       \
        if (END) {                                                                                 \
            node->prev->next = node->next;                                                         \
            node->next->prev = node->prev;                                                         \
        } else {                                                                                   \
            if (node->prev) node->prev->next = node->next;                                         \
            else l->head = node->next;                                                             \
            if (node->next) node->next->prev = node->prev;                                         \
            else l->tail = node->prev;                                                             \
        }                                                                                          \
        free(node);                                                                                \
    }                                                                                              \
    static inline size_t ListT##_Count(const struct ListT* l) {                                    \
        size_t c = 0;                                                                              \
        for (struct NodeT* n = ListT##_First(l); n != END; n = n->next) c++;                       \
        return c;                                                                                  \
    }                                                                                              \
    static inline void ListT##_Free(struct ListT* l) {                                             \
        struct NodeT* n = ListT##_First(l);                                                        \
        while (n != END) { struct NodeT* nx = n->next; free(n); n = nx; }                          \
        if (END) { END->next = END; END->prev = END; }                                             \
        else { l->head = 0; l->tail = 0; }                                                         \
    }

// ponytail: sentinel mode of DEFINE_LINKED_LIST ignores List.head/tail (the sentinel owns
// the ends); the fields are kept only so the NULL branch still compiles.

#endif
