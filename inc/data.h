typedef struct{
    List *feats, *cls, *uFeats;
    int numEntries;
} Data;

Data *createData();
void deleteData(Data *data);