#ifndef SORT_GRAPH_VIS
#define SORT_GRAPH_VIS

#include "base/core.h"

#define quick_sort_single(array, l, r, p) (do { \
    do { *l++; } while (*l <= *r && array[*l] < p); \
    do { *r--; } while (*r >= *l && array[*r] > p); \
    if (l < r) MemorySwap(&array[*l], &array[*r]); \
} while(0))

void QuickSort(S32Array array);
void MergeSort(S32Array array);
void InsertionSort(S32Array array);
void BubbleSort(S32Array array);

#endif
