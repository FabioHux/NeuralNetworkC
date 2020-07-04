#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "list.h"
#include "data.h"
#include "preproc.h"

List *getDocFreq(Data *data){
    List *featCount = createDataList(data->uFeats->size, sizeof(int), NULL);

    if(featCount == NULL){
        return NULL;
    }

    int i = 0, j = 0;

    for(;i < data->uFeats->size; i++){
        append(featCount, &j);
    }

    for(i = 0; i < data->numEntries; i++){
        List *list = *((List **) data->feats->data + i);
        for(j = 0; j < list->size; j++){
            int *inc = (int *) get(featCount, indexOf(data->uFeats, get(list, j)));
            *inc += 1;
        }
    }

    return featCount;
}

Data *extractData(char *filename){
    FILE *f = fopen(filename, "r");

    if(f == NULL){
        printf("File not found, exiting\n");
        exit(0);
    }
    char *line = (char *) malloc(sizeof(char) * 40000);

    if(line == NULL){
        printf("Insufficient space required to extract data. Exiting.\n");
        exit(0);
    }

    Data *data = createData();

    if(data == NULL){
        printf("Could not create data. Exiting.\n");
        exit(0);
    }

    while(fgets(line, 40000,f) != NULL){
        List *l = createDataList(200, sizeof(int), intCmp);
        append(data->feats, &l);
        int num = *line - '0';
        append(data->cls, &num);

        num = 0;
        char *chr = line + 1;
        while(*(++chr) != '\n'){
            if(*chr != ' '){
                num = (num<<3) + (num<<1) + *chr - '0';
            }else{
                insertSorted(l, &num);
                //append(l, &num);
                int propInd = closestIndexOf(data->uFeats, &num);
                if(propInd >= data->uFeats->size || *((int *) get(data->uFeats, propInd)) != num){
                    insert(data->uFeats, &num, propInd);
                }
                num = 0;
            }
        }
    }
    data->numEntries = data->cls->size;
    free(line);
    fclose(f);
    return data;
}

int binTransform(Data *data, float low, float high){
    int lowF = (int)(low * data->numEntries), highF = (int)(high * data->numEntries);
    
    //printf("Low: %d; High: %d\n", lowF, highF);

    List *docFreq = getDocFreq(data);
    //printf("Done getting freqs\n");
    if(docFreq != NULL){
        int i = data->uFeats->size - 1;
        for(;i >= 0; i--){
            int frequency = *((int *) get(docFreq, i));
            if(frequency < lowF || frequency > highF){
                free(removeRet(data->uFeats, i));
            }
        }
        deleteDataList(docFreq);

        List *repL = createDataList(data->uFeats->size, sizeof(int), NULL);

        int j = 0;
        if(repL == NULL){
            printf("Error occurred in binTransform. Exiting.\n");
            exit(0);
        }else{
            for(i = 0; i < repL->len; i++){
                append(repL, &j);
            }
        }

        for(i = 0; i < data->numEntries; i++){
            List *list = *((List **) get(data->feats, i));
            for(j = 0; j < repL->size; j++){
                if(indexOf(list, get(data->uFeats, j)) == -1){
                    *((int *) get(repL, j)) = 0;
                }else{
                    *((int *) get(repL, j)) = 1;
                }
            }

            int minSize = repL->size;
            if(minSize > list->size){
                minSize = list->size;
            }

            for(j = 0; j < minSize; j++){
                *((int *) get(list, j)) = *((int *) get(repL, j));
            }

            if(minSize < repL->size){
                for(;j < repL->size; j++){
                    append(list, get(repL, j));
                }
            }else{
                list->size = repL->size;
            }


        }

        deleteDataList(repL);
    }else{
        return 0;
    }

    return 1;
}