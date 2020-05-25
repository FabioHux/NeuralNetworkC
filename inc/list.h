typedef struct{
    void *data;
    int size, len;
    char dType;
} List;

static const char *types[] = {"int", "long", "float", "double", "char", "string", "list"};

char append(List *list, void *value);
char insert(List *list, void *value, int index);
char insertSorted(List *list, void *value);
void insertionResize(List *list);
void *removeRet(List *list, int index);
int removeNoRet(List *list, int index);
void *get(List *list, int index);
char set(List *list, int index, void *value);
int indexOf(List *list, void *value);
int closestIndexOf(List *list, void *value);
int cmpVal(void *a, void *b, int flag);
List *createDataList(int len, char *type);
void deleteDataList(List *list);
size_t sizeType(int flag);
void copyVal(void *src, void *trg, int flag);
