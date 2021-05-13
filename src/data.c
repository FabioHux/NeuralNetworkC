/**
 * ***INSERT DESCRIPTION***
 * 
 * Author: Fabio Hux
 * 
 * Date: 5/24/2020
 * 
 * Data last edited: 7/5/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include "libremodel.h"
#include "data.h"

/**
 * Function to create the data struct.
 * 
 * Function will return a Data pointer referencing a Data value with initialized components. The cls, feats, and uFeats lists will have a starting length of 1. The feats list is a list of lists, each pointing to one entry.
 * The way to access a specific row in feats would be: (*(data->feats->data + row)->data + col).
 * 
 * Function will return NULL on failure. No memory leaks will (should??) occur.
 * 
 * NOTE: The function could cause the process to exit in the case it cannot allocate memory.
 */

Data *createData(){

    Data *data = (Data *) calloc(1,sizeof(Data));

    if(data != NULL){
        data->cls = listCreate(100, sizeof(Matrix *), NULL, nulled_matrix_destroy);
        if(data->cls != NULL){
            data->uFeats = listCreate(1000, sizeof(double), list_double_cmp, NULL);
            if(data->uFeats != NULL){
                data->feats = listCreate(100,sizeof(Matrix *),NULL, nulled_matrix_destroy);
                if(data->feats != NULL){
                    return data;
                }
            }
        }
    }else{
        printf("An error occurred with malloc in createData on creating the data. Exiting.\n");
        exit(0);
    }

    //printf("Failed\n");
    deleteData(data);
    return NULL;
}

/**
 * Function to delete a Data struct.
 * 
 * This function will clear the data struct and all of its components by freeing each component.
 * If components of the struct are NULL such as data->feats or data->uFeats the function will still behave normally and not attempt to free those components.
 */

void deleteData(Data *data){
    if(data == NULL) return;

    listDestroy(data->feats);
    listDestroy(data->cls);
    listDestroy(data->uFeats);

    free(data);
}
/* TO FIX LATER
DataPack *createDataPack(){
    DataPack *datapack = (DataPack *) calloc(sizeof(DataPack),1);
    if(datapack != NULL){
        datapack->train = createData();
        if(datapack->train != NULL){
            datapack->train->feats->destroy = NULL;
            datapack->valid = createData();
            if(datapack->valid != NULL){
                datapack->valid->feats->destroy = NULL;
                return datapack;
            }
        }
    }

    return NULL;
}
void deleteDataPack(void *datapack){
    if(datapack == NULL) return;

    deleteData((*((DataPack **)datapack))->train);
    deleteData((*((DataPack **)datapack))->valid);
    free(*((DataPack **)datapack));
}

List *createCrossVal(Data *data, int folds){
    if(folds < 2 || folds > data->numEntries) return NULL;

    List *crossVals = createDataList(folds, sizeof(DataPack *), NULL, deleteDataPack);

    if(crossVals != NULL){
        int s = data->numEntries / folds;
        int i = 0;
        for(; i < folds; i++){
            DataPack * dp= createDataPack();
            if(dp == NULL){
                printf("Failed to make datapack. Exiting.\n");
                exit(0);
            }

            dp->train->numEntries = data->numEntries - s;
            dp->valid->numEntries = s;
            append(crossVals, &dp);
        }

        int j = -1;
        int k = 0;
        for(i = 0; i < data->numEntries; i++){
            if(i % s == 0){
                j++;
            }
            
            for(k = 0; k < folds; k++){
                if(k == j){
                    append((*((DataPack **)get(crossVals,k)))->valid->feats, get(data->feats, i));
                    append((*((DataPack **)get(crossVals,k)))->valid->cls, get(data->cls, i));
                }else{
                    append((*((DataPack **)get(crossVals,k)))->train->feats, get(data->feats, i));
                    append((*((DataPack **)get(crossVals,k)))->train->cls, get(data->cls, i));
                }
            }
        }
    }
    return crossVals;
}
void deleteCrossVal(List *crossVals){
    if(crossVals == NULL || crossVals->data == NULL) return;

    int i = 0;
    for(;i < crossVals->size;i++){

    }
}
*/
