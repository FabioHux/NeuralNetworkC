#include <stdio.h>
#include <string.h>
#include <time.h>
#include "list.h"
#include "data.h"
#include "neural.h"
#include "preproc.h"
#include "matrix.h"

#define N 10

int sorter(void *a, void *b){
    return strcmp(*((char **)a), *((char **)b));
}

int testing(){
    printf("Hello, creating list.\n");
    char *arb[N] = {"Banana", "Balls", "Bananas", "DragonBall", "1234", "Hello", "Discord", "Volleyball", "CS", "Let's hope it works"};
    List *list = createDataList(1, sizeof(char *), sorter, NULL);
    int i = N - 1;
    for(; i >= 0; i--){
        if(!insertSorted(list, (void *) &arb[i])){
            printf("Failure in appending: %d\n", i);
        }
    }

    for(i = 0; i < listGetSize(list); i++){
        printf("Value at %d: %s\n", i, *((char **) get(list, i)));
    }

    return 0;
}

void printList(List *list){
    int i = 0;

    printf("%lf", DER(double, get(list, i)));

    while(++i < listGetSize(list)){
        printf(",%lf", DER(double, get(list, i)));
    }

    printf("\n");
}

void networkTest(){
    Data *data = createData();

    int i = 0;
    for(i = 0; i < 2; i++){
        int j;
        for(j = 0; j < 2; j++){
            List *list = createDataList(2, sizeof(double), NULL, NULL);

            double vals[2] = {i, j};
            printf("%d, %d\n", i , j);
            append(list, vals);
            append(list, vals + 1);

            append(data->feats, &list);

            int num = i * 2 + j;

            append(data->cls, &num);
        }
    }

    int layers[4] = {2,100,150,20};

    NeuralNetwork *network = createNN(layers, 4, .001,100, .1,0,"output.json", 1000);

    trainNetwork(network, data);
}

void matrixTest(){
    double aList[6] = {1,2,3,4,5,6};
    double bList[6] = {6,5,4,3,2,1};
    //double cList[3] = {-.47,-1.17,-.22};
    
    Matrix *a = createMatrix(2,3, aList, 6);
    Matrix *b = createMatrix(3,2, bList, 6);
    Matrix *c = createMatrix(2,3, NULL, 6);

    FILE *f = fopen("output.json", "w");

    matSub(a,b,c,2);
    fprintf(f, "{\"Matrix A\":[");
    printMatrixJSON(a, f);
    fprintf(f, "],\"Matrix B\":[");
    printMatrixJSON(b, f);
    fprintf(f, "],\"Matrix C\":[");
    printMatrixJSON(c, f);
    fprintf(f, "]}");
}

double accuracy(List *exp, List *res){
    if(exp == NULL || res == NULL || exp->eSize != sizeof(int) || exp->eSize != res->eSize || exp->size != res->size) return 0;

    double accuracy = 0;

    int *expected = (int *) exp->data, *resulting = (int *) res->data;
    int *stop = expected + exp->size;

    while(expected != stop){
        if(*expected == *stop){
            accuracy += 1;
        }

        expected++;
        resulting++;
    }

    return accuracy / exp->size;
}

double f1Score(List *exp, List *res){
    if(exp == NULL || res == NULL || exp->eSize != sizeof(int) || exp->eSize != res->eSize || exp->size != res->size) return 0;

    double tp = 0, fp = 0, fn = 0;

    int *expected = (int *) exp->data, *resulting = (int *) res->data;
    int *stop = expected + exp->size;

    while(expected != stop){
        //printf("\n%d vs %d", *expected, *resulting);
        if(*expected){
            if(*resulting) tp += 1;
            else fn += 1;
        }else if(*resulting) fp += 1;

        expected++;
        resulting++;
    }
    //printf("\n");
    if(tp == 0 && fp == 0 && fn == 0) return 1;

    return (2 * tp) / ((2 * tp) + fn + fp);
}


int main(){
    //networkTest();
    //matrixTest();
    //return 0;
    clock_t secs;    
    secs = clock();
    printf("Hi");

    Data *data = extractData("training.txt");
    binTransform(data, .025, .08);
    const int numCV = 8;
    List *crossVals = createCrossVal(data, numCV);
    
    double average = 0;
    for(int i = 0; i < numCV; i++){
        printf("***BEGINNING CROSSVAL %d***\n\n***TRAINING NETWORK***\n\n",i);
        int layers[] = {listGetSize(LIST_DER(List *, listGet(data->feats, 0))),10,10,10,10,10,1};
        NeuralNetwork *network = createNN(layers, 7, 0.04, 200, .1, 0, "output.json", (int) clock());
        trainNetwork(network, (LIST_DER(DataPack *, listGet(crossVals, i)))->train);
        printf("\n***NETWORK TRAINED. TESTING NETWORK***\n\n");
        List *results = classifyNetwork(network, (LIST_DER(DataPack *, listGet(crossVals, i)))->valid);
        double score = f1Score((LIST_DER(DataPack *, listGet(crossVals, i)))->valid->cls, results);
        average += score;
        printf("\n***RESULTING SCORE: %.3lf***\n\n***ENDED***\n\n", score);
        listDestroy(results);
        destroyNN(network);
    }
    printf("\n***FINAL AVERAGE RESULT: %.3lf***\n\n",average/numCV);
    printf("%d\n", listGetSize(LIST_DER(List *, listGet(data->feats, 0))));
    listDestroy(crossVals);
    deleteData(data);

    printf("\nTime to complete: %lf\n", (double)(clock()-secs) / CLOCKS_PER_SEC);

    return 0;
}
