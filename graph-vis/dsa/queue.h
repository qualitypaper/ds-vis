#ifndef GRAPH_VIS_QUEUE
#define GRAPH_VIS_QUEUE

#include "list.h"

typedef struct AQueue { struct List list; } AQueue;

struct AQueue AQ_Init(Arena* arena, size_t cap);
void          AQ_Enqueue(int val, struct AQueue* q);
int           AQ_Dequeue(struct AQueue* q);
int           AQ_IsEmpty(struct AQueue* q);

struct FQueue {
	int* arr;
	size_t head, tail, len, cap;
};

struct FQueue FQ_Init(Arena* arena, size_t cap);
void          FQ_Enqueue(int val, struct FQueue* q);
int           FQ_Dequeue(struct FQueue* q);
int           FQ_IsEmpty(struct FQueue* q);
int           FQ_IsFull(struct FQueue* q);

#define DEFINE_QUEUE(T, Name)                                                                         \
typedef struct Name { T* arr; size_t head, tail, len, cap; Arena* arena; } Name;                                   \
static inline struct Name Name##_Init(Arena* arena, size_t cap) {                                   \
    struct Name q = { PushArray(arena, T, cap), 0, 0, 0, cap, arena };                              \
    return q;                                                                                         \
}                                                                                                     \
static inline void Name##_Grow(struct Name* q) {                                                      \
    size_t newcap = (size_t)(q->cap * GROWTH_FACTOR);                                                 \
    if (newcap <= q->cap) newcap = q->cap + 1; /* 1.6 floors to 1 for tiny caps */                    \
    T* newarr = PushArray(q->arena, T, newcap);                                                       \
    for (size_t i = 0; i < q->len; i++)                                                               \
        newarr[i] = q->arr[(q->head + i) % q->cap];                                                  \
    q->arr = newarr; q->head = 0; q->tail = q->len; q->cap = newcap;                                 \
}                                                                                                     \
static inline void Name##_Enqueue(T val, struct Name* q) {                                            \
    if (q->len >= q->cap) Name##_Grow(q);                                                             \
    q->arr[q->tail] = val;                                                                            \
    q->tail = (q->tail + 1) % q->cap;                                                                 \
    q->len++;                                                                                         \
}                                                                                                     \
static inline T Name##_Dequeue(struct Name* q) {                                                      \
    T val = q->arr[q->head];                                                                          \
    q->head = (q->head + 1) % q->cap;                                                                 \
    q->len--;                                                                                         \
    return val;                                                                                       \
}                                                                                                     \
static inline int Name##_IsEmpty(const struct Name* q) { return q->len == 0; }


DEFINE_QUEUE(F32, FlQueue);

#endif
