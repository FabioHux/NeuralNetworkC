#ifndef LIST_CONST
#define LIST_CONST

// #include <stdlib.h>
// 
// #define LIST_DER(TYPE, X) *((TYPE *) X)
// 
// typedef struct _list List;
// 
// 
// int intCmp(void *a, void *b);
// int dblCmp(void *a, void *b);
// void listDestroyer(void *element);
// 
// 
// char listAppend(List *list, void *value);
// char listInsert(List *list, void *value, int index);
// char listInsertSorted(List *list, void *value);
// void *listRemoveRet(List *list, int index);
// void *listGet(List *list, int index);
// char listSet(List *list, int index, void *value);
// int listIndexOf(List *list, void *value);
// int listCmpVal(void *a, void *b, List *list); 
// void listDestroy(List *list);
// int listGetSize(List *list);
// int listGetESize(List *list);
// 
// 
// 
// List *listCreate(int len, size_t size, int (*cmp)(void *, void *), void (*destroy)(void *));

struct _list{
    void *data;
    int (*cmp)(void *, void *);
    void (*destroy)(void *);
    int size, len;
    int eSize;
    char sorted;
};

#endif
