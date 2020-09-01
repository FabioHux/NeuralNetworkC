#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "data.h"
#include "preproc.h"

List *getDocFreq(Data *data){
    List *featCount = createDataList(data->uFeats->size, sizeof(int), NULL, NULL);

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
        //printf("Here\n");
        List *l = createDataList(200, sizeof(double), dblCmp, NULL);
        if(l == NULL){
            printf("\tFailed??");
        }
        if(!append(data->feats, &l)){
            printf("\tDidn't tho???");
        }
        int num = *line - '0';
        append(data->cls, &num);

        num = 0;
        double dnum = 0;
        char *chr = line + 1;
        while(*(++chr) != '\n'){
            if(*chr != ' '){
                num = (num<<3) + (num<<1) + *chr - '0';
            }else{
                dnum = num;
                insertSorted(l, &dnum);
                //append(l, &num);
                int propInd = closestIndexOf(data->uFeats, &dnum);
                if(propInd >= data->uFeats->size || *((double *) get(data->uFeats, propInd)) != dnum){
                    insert(data->uFeats, &dnum, propInd);
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

        List *repL = createDataList(data->uFeats->size, sizeof(double), NULL, NULL);

        double k = 0;
        if(repL == NULL){
            printf("Error occurred in binTransform. Exiting.\n");
            exit(0);
        }else{
            for(i = 0; i < repL->len; i++){
                append(repL, &k);
            }
        }
        
        int j;
        for(i = 0; i < data->numEntries; i++){
            List *list = *((List **) get(data->feats, i));
            for(j = 0; j < repL->size; j++){
                if(indexOf(list, get(data->uFeats, j)) == -1){
                    *((double *) get(repL, j)) = 0;
                }else{
                    *((double *) get(repL, j)) = 1;
                }
            }

            int minSize = repL->size;
            if(minSize > list->size){
                minSize = list->size;
            }

            for(j = 0; j < minSize; j++){
                *((double *) get(list, j)) = *((double *) get(repL, j));
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