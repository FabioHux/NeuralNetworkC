#include <stdio.h>
#include <string.h>
#include <time.h>
#include "list.h"
#include "data.h"
#include "preproc.h"

#define N 10

int sorter(void *a, void *b){
    return strcmp(*((char **)a), *((char **)b));
}

int testing(){
    printf("Hello, creating list.\n");
    char *arb[N] = {"Banana", "Balls", "Bananas", "DragonBall", "1234", "Hello", "Discord", "Volleyball", "CS", "Let's hope it works"};
    List *list = createDataList(1, sizeof(char *), sorter);
    int i = N - 1;
    for(; i >= 0; i--){
        if(!insertSorted(list, (void *) &arb[i])){
            printf("Failure in appending: %d\n", i);
        }
    }

    for(i = 0; i < list->size; i++){
        printf("Value at %d: %s\n", i, *((char **) get(list, i)));
    }

    return 0;
}

void printList(List *list){
    int i = 0;

    printf("%d", *((int *)(list->data) + i));

    while(++i < list->size){
        printf(",%d", *((int *)(list->data) + i));
    }

    printf("\n");
}

int main(){
    clock_t secs;

    secs = clock();
    Data *data = extractData("training.txt");

    //printList(*((List **)get(data->feats, 0)));

    printf("Numentries: %d\n", data->numEntries);

    printf("Size of uFeats before: %d\n", data->uFeats->size);

    binTransform(data, .025, .1);

    //printList(data->uFeats);
    
    printf("Size of uFeats after: %d\n", data->uFeats->size);

    printf("Size of transformed first entry: %d\n", (*((List **) get(data->feats, 0)))->size);

    //printList(*((List **)get(data->feats, 0)));

    deleteData(data);

    printf("Time to complete: %lf\n", (double)(clock()-secs));

    return 0;

    //return testing();
}