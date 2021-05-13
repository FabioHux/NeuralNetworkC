#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libremodel.h>
#include "data.h"
#include "preproc.h"

#define N 10

int sorter(void *a, void *b){
    return strcmp(*((char **)a), *((char **)b));
}

int testing(){
    printf("Hello, creating list.\n");
    char *arb[N] = {"Banana", "Balls", "Bananas", "DragonBall", "1234", "Hello", "Discord", "Volleyball", "CS", "Let's hope it works"};
    List *list = listCreate(1, sizeof(char *), sorter, NULL);
    int i = N - 1;
    for(; i >= 0; i--){
        if(!listInsertSorted(list, (void *) &arb[i])){
            printf("Failure in appending: %d\n", i);
        }
    }
//     list->size = 5;

    for(i = 0; i < listGetSize(list); i++){
        printf("Value at %d: %s\n", i, *((char **) listGet(list, i)));
    }
    
    printf("Index of %s = %d\n", arb[3], listIndexOf(list, &arb[3]));
    
    return 0;
}

void printList(List *list){
    int i = 0;

    printf("%.0lf", LIST_DER(double, listGet(list, i)));

    while(++i < listGetSize(list)){
        printf(", %.0lf", LIST_DER(double, listGet(list, i)));
    }

    printf("\n");
}

// void networkTest(){
//     Data *data = createData();
// 
//     int i = 0;
//     for(i = 0; i < 2; i++){
//         int j;
//         for(j = 0; j < 2; j++){
//             List *list = listCreate(2, sizeof(double), NULL, NULL);
// 
//             double vals[2] = {i, j};
//             printf("%d, %d\n", i , j);
//             listAppend(list, vals);
//             listAppend(list, vals + 1);
// 
//             listAppend(data->feats, &list);
// 
//             int num = i * 2 + j;
// 
//             listAppend(data->cls, &num);
//         }
//     }
// 
//     int layers[4] = {2,100,150,20};
// 
//     NeuralNetwork *network = createNN(layers, 4, .001,100, .1,0,"output.json", 1000);
// 
//     trainNetwork(network, data);
// }

void matrixTest(){
    double aList[6] = {1,2,3,4,5,6};
    double bList[6] = {6,5,4,3,2,1};
    //double cList[3] = {-.47,-1.17,-.22};
    
    Matrix *a = matrixCreate(2,3, aList, 6);
    Matrix *b = matrixCreate(3,2, bList, 6);
    Matrix *c = matrixCreate(3,3, NULL, 9);

    FILE *f = fopen("output.json", "w");

    if((c = matrixMul(a,b,c,MATRIX_A_TRANS | MATRIX_B_TRANS | MATRIX_RESULT_SUB)) == NULL){
        printf("Error occurred.\n");
    }
    fprintf(f, "{\"Matrix A\":[");
    matrixPrintJSON(a, f);
    fprintf(f, "],\"Matrix B\":[");
    matrixPrintJSON(b, f);
    fprintf(f, "],\"Matrix C\":[");
    matrixPrintJSON(c, f);
    fprintf(f, "]}");
}

double accuracy(List *exp, List *res){
    if(exp == NULL || res == NULL || listGetESize(exp) != sizeof(int) || listGetESize(exp) != listGetESize(res) || listGetSize(exp) != listGetSize(res)) return 0;

    double accuracy = 0;

    int *expected = (int *) listGet(exp, 0), *resulting = (int *) listGet(res, 0);
    int *stop = expected + listGetSize(exp);

    while(expected != stop){
        if(*expected == *stop){
            accuracy += 1;
        }

        expected++;
        resulting++;
    }

    return accuracy / listGetSize(exp);
}

double f1Score(List *exp, List *res){
    if(exp == NULL || res == NULL || listGetESize(exp) != sizeof(int) || listGetESize(exp) != listGetESize(res) || listGetSize(exp) != listGetSize(res)) return 0;

    double tp = 0, fp = 0, fn = 0;

    int *expected = (int *) listGet(exp, 0), *resulting = (int *) listGet(res, 0);
    int *stop = expected + listGetSize(exp);

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
    
   // matrixTest();
//     testing();

      Data *data = extractData("training.txt");
      binTransform(data, .025, .03);
     
     printf("***SIZE AFTER TRANSFORM: %d***\n", listGetSize(data->uFeats));
    printf("\nTime to complete stage 1: %lf\n", (double)(clock()-secs) / CLOCKS_PER_SEC);
     //printList(data->uFeats);
    NeuralNetworkSolver *solver = neural_network_solver_sgd(.05, 1);
     NeuralNetwork *network = neural_network_create(solver, 1, "output2.json", 4435621);
     
     int layers[] = {100, 75, 50, 25};
     neural_network_add_input_layer(network, matrixGetN(LIST_DER(Matrix *, listGet(data->feats, 0))));
     int i;
     for(i = 0; i < 4; i++){
        neural_network_add_hidden_layer(network, layers[i], ACTIV_FUNC_SIGMOID);
     }
     neural_network_add_output_layer(network, matrixGetN(LIST_DER(Matrix *, listGet(data->cls, 0))));
    
     neural_network_train(network, data->feats, data->cls);
     
//     const int numCV = 8;
//     List *crossVals = createCrossVal(data, numCV);
//     
//     double average = 0;
//     for(int i = 0; i < numCV; i++){
//         printf("***BEGINNING CROSSVAL %d***\n\n***TRAINING NETWORK***\n\n",i);
//         int layers[] = {listGetSize(LIST_DER(List *, listGet(data->feats, 0))), 50, 25,1};
//         NeuralNetwork *network = createNN(layers, 7, 0.04, 200, .1, 0, "output.json", (int) clock());
//         trainNetwork(network, (LIST_DER(DataPack *, listGet(crossVals, i)))->train);
//         printf("\n***NETWORK TRAINED. TESTING NETWORK***\n\n");
//         List *results = classifyNetwork(network, (LIST_DER(DataPack *, listGet(crossVals, i)))->valid);
//         double score = f1Score((LIST_DER(DataPack *, listGet(crossVals, i)))->valid->cls, results);
//         average += score;
//         printf("\n***RESULTING SCORE: %.3lf***\n\n***ENDED***\n\n", score);
//         listDestroy(results);
//         destroyNN(network);
//     }
//     printf("\n***FINAL AVERAGE RESULT: %.3lf***\n\n",average/numCV);
//     printf("%d\n", listGetSize(LIST_DER(List *, listGet(data->feats, 0))));
//     listDestroy(crossVals);
//     deleteData(data);

    printf("\nTime to complete everything: %lf\n", (double)(clock()-secs) / CLOCKS_PER_SEC);

    return 0;
}
