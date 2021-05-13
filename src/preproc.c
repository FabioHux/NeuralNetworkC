#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "libremodel.h"
#include "data.h"
#include "preproc.h"

List *getDocFreq(Data *data){
    //Create list to count the number of features in the dataset for each feature
    List *featCount = listCreate(listGetSize(data->uFeats), sizeof(int), NULL, NULL);

    if(featCount == NULL){
        return NULL;
    }

    int i = 0, j = 0, k;
    //Set the list to 0 first of all
    for(;i < listGetSize(data->uFeats); i++){
        listAppend(featCount, &j);
    }
    
    //For every single entry
    double value;
    for(i = 0; i < data->numEntries; i++){
        Matrix *list = *((Matrix **) listGet(data->feats, i));
        int n = matrixGetN(list);
        int m = matrixGetM(list);
        //For every element in the entry
        for(j = 0; j < n; j++){
            for(k = 0; k < m; k++){
                value = matrixGetValue(list, j, k);
                //Increment the counter at the position the current feature exists inside of data->uFeats
                int *inc = (int *) listGet(featCount, listIndexOf(data->uFeats, &value));
                (*inc)++;
            }
        }
    }

    return featCount;
}

Data *extractData(char *filename){
    //Open file
    FILE *f = fopen(filename, "r");
    if(f == NULL){
        printf("File not found, exiting\n");
        exit(0);
    }
    //Create a buffer
    char *line = (char *) malloc(sizeof(char) * 40000);
    if(line == NULL){
        printf("Insufficient space required to extract data. Exiting.\n");
        exit(0);
    }
    //Create a Data struct to hold the data
    Data *data = createData();
    if(data == NULL){
        printf("Could not create data. Exiting.\n");
        exit(0);
    }
    
    //While there's a new entry... extract that data
    while(fgets(line, 40000,f) != NULL){
        //printf("Here\n");
        //List here will hold all of the features it finds, to be sent to a Matrix and inserted into the data
        List *l = listCreate(200, sizeof(double), list_double_cmp, NULL);
        if(l == NULL){
            printf("\tFailed??");
        }
        /*if(!listAppend(data->feats, &l)){
            printf("\tDidn't tho???");
        }*/
        //First get the class
        int num = *line - '0';
        Matrix *cls_mat = matrixCreate(1,1, NULL, 1);
        assert(cls_mat != NULL);
        matrixSetMat(cls_mat, num);
        listAppend(data->cls, &cls_mat);
    
        //Keep going through the file and extract each feature, inserting it into l
        num = 0;
        double dnum = 0;
        char *chr = line + 1;
        while(*(++chr) != '\n'){
            if(*chr != ' '){
                num = (num<<3) + (num<<1) + *chr - '0';
            }else{
                dnum = num;
                //Inserting sorted because why not? (I can't remember why I did it... will update if I remember to)
                listInsertSorted(l, &dnum);
                //append(l, &num);
                //If the feature has never been seen before, insert it into uFeats (unique features)
                if(listIndexOf(data->uFeats, &dnum) < 0){
                    listInsertSorted(data->uFeats, &dnum);
                }
                num = 0;
            }
        }
        
        //Transfer the information of l to a matrix and append that to data->feats
        Matrix *feat_mat = matrixCreate(listGetSize(l), 1, listGet(l, 0), listGetSize(l));
        assert(feat_mat != NULL);
        listAppend(data->feats, &feat_mat);
    }
    data->numEntries = listGetSize(data->cls);
    free(line);
    fclose(f);
    return data;
}

int binTransform(Data *data, float low, float high){
    int lowF = (int)(low * data->numEntries), highF = (int)(high * data->numEntries);
    
    printf("Low: %d; High: %d\n", lowF, highF);
    //Get the document frequency (note that it's the same dimension as data->uFeats
    //Since it must represent the frequency of each unique feature
    List *docFreq = getDocFreq(data);
    //printf("Done getting freqs\n");
    if(docFreq != NULL){
        //Doing it backwards because updating a dynamic list that way is **much** faster
        int i = listGetSize(data->uFeats) - 1;
        for(;i >= 0; i--){
            int frequency = *((int *) listGet(docFreq, i));
            if(frequency < lowF || frequency > highF){
                free(listRemoveRet(data->uFeats, i));
            }
        }
        listDestroy(docFreq); //I don't need this anymore.. by now I have all the unique features that I need
        int uFeatsSize = listGetSize(data->uFeats);
        //Replacement list
        List *repL = listCreate(uFeatsSize, sizeof(double), NULL, NULL);

        double k = 0;
        if(repL == NULL){
            printf("Error occurred in binTransform. Exiting.\n");
            exit(0);
        }else{
            //Each replacement list is exactly the same size so I'm just setting it to be the same size as uFeatsSize
            for(i = 0; i < uFeatsSize; i++){
                listAppend(repL, &k);
            }
        }
        
        //Now replace each and every entry with a new, binary one that represents the existence of a feature or not
        int j;
        for(i = 0; i < data->numEntries; i++){
            //Get the current entry
            Matrix *list = LIST_DER(Matrix *, listGet(data->feats, i));
            //Iterate through it
            matrixResetIter(list);
            
            double value;
            while(!isnan(value = matrixGetNext(list, -1))){
                //If the feature exists in the current list
                //then set repL to 1 at the index in which it was found inside uFeats
                int index = listIndexOf(data->uFeats, &value);
                if(index > -1){
                    LIST_DER(double, listGet(repL, index)) = 1;
                }
            }
            //Swap the old matrix with the new one
            Matrix *new = matrixCreate(listGetSize(repL), 1, listGet(repL, 0), listGetSize(repL));
            assert(new != NULL);
            matrixDestroy(list, 0);
            listSet(data->feats, i, &new);
            
            //matrixPrint(new);
            //printf("\n**********************************************************************************************\n\n");
            
            //reset repL to be all 0s for the next entry list
            for(j = 0; j < uFeatsSize; j++){
                listSet(repL, j, &k);
            }
            
            /*
            List *list = *((List **) listGet(data->feats, i));
            for(j = 0; j < listGetSize(repL); j++){
                if(listIndexOf(list, listGet(data->uFeats, j)) == -1){
                    *((double *) listGet(repL, j)) = 0;
                }else{
                    *((double *) listGet(repL, j)) = 1;
                }
            }

            int minSize = listGetSize(repL);
            if(minSize > listGetSize(list)){
                minSize = listGetSize(list);
            }

            for(j = 0; j < minSize; j++){
                *((double *) listGet(list, j)) = *((double *) listGet(repL, j));
            }

            if(minSize < listGetSize(repL)){
                for(;j < listGetSize(repL); j++){
                    listAppend(list, listGet(repL, j));
                }
            }else{
                int i = listGetSize(list);
                while(i > minSize)
                    listRemoveRet(list, --i);
            }*/
            
        }
        listDestroy(repL);
    }else{
        return 0;
    }

    return 1;
}
