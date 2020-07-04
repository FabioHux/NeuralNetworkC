/**
 * ***INSERT DESCRIPTION***
 * 
 * Author: Fabio Hux
 * 
 * Date: 5/24/2020
 * 
 * Data last edited: 7/3/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include "list.h"
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
        data->cls = createDataList(100, sizeof(int), NULL);
        if(data->cls != NULL){
            data->uFeats = createDataList(90000, sizeof(int), intCmp);
            if(data->uFeats != NULL){
                data->feats = createDataList(100,sizeof(List *),NULL);
                if(data->feats == NULL){
                    deleteData(data);
                    return NULL;
                }
            }else{
                deleteData(data);
                return NULL;
            }
        }else{
            deleteData(data);
            return NULL;
        }
    }else{
        printf("An error occurred with malloc in createData on creating the data. Exiting.\n");
        exit(0);
    }

    return data;
}

/**
 * Function to delete a Data struct.
 * 
 * This function will clear the data struct and all of its components by freeing each component.
 * If components of the struct are NULL such as data->feats or data->uFeats the function will still behave normally and not attempt to free those components.
 */

void deleteData(Data *data){
    if(data == NULL) return;

    int i;

    for(i = 0; i < data->numEntries; i++){
        deleteDataList(*((List **) get(data->feats, i)));
    }

    deleteDataList(data->feats);
    deleteDataList(data->cls);
    deleteDataList(data->uFeats);

    free(data);
}