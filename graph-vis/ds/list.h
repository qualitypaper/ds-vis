#ifndef GRAPH_VIS_LIST
#define GRAPH_VIS_LIST

#include <stdlib.h>
#include <string.h>

#define GROWTH_FACTOR 1.6

struct List {
	int* arr;
	size_t len;
	size_t cap;
};

struct List L_Init(size_t cap);
void        L_Grow(struct List* list);
void        L_Add(int val, struct List* list);
void        L_Del(size_t index, struct List* list);

#define DEFINE_LIST(T, Name)                                                                          \
struct Name { T* arr; size_t len, cap; };                                                             \
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
    memmove(&l->arr[i], &l->arr[i + 1], (l->len - i - 1) * sizeof(T));                               \
    l->len--;                                                                                         \
}

#endif
