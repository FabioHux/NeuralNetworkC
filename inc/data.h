#include "list.h"

typedef struct{
    List *feats, *cls, *uFeats;
    int numEntries;
} Data;

Data *createData(int entries);
void deleteData(Data *data);