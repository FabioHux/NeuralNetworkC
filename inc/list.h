#ifndef LIST_CONST
#define LIST_CONST

#include <stdlib.h>

typedef struct{
    void *data;
    int (*cmp)(void *, void *);
    void (*destroy)(void *);
    int size, len;
    int eSize;
} List;


int intCmp(void *a, void *b);
int dblCmp(void *a, void *b);
void listDestroyer(void *element);


char append(List *list, void *value);
char insert(List *list, void *value, int index);
char insertSorted(List *list, void *value);
void *removeRet(List *list, int index);
//int removeNoRet(List *list, int index);
void *get(List *list, int index);
char set(List *list, int index, void *value);
int indexOf(List *list, void *value);
int cmpVal(void *a, void *b, List *list); 
void deleteDataList(List *list);
int closestIndexOf(List *list, void *value);


List *createDataList(int len, size_t size, int (*cmp)(void *, void *), void (*destroy)(void *));

#endif
