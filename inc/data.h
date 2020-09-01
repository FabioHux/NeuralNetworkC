#ifndef DATA_CONST
#define DATA_CONST
#include "list.h"

typedef struct{
    List *feats, *cls, *uFeats;
    int numEntries;
} Data;

typedef struct{
    Data *train, *valid;
} DataPack;

Data *createData();
void deleteData(Data *data);

DataPack *createDataPack();
void deleteDataPack(void *datapack);

List *createCrossVal(Data *data, int folds);
void deleteCrossVal(List *crossVals);
#endif