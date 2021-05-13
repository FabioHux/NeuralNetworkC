#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "libremodel.h"
#include "neural_network/components/activ_func.h"
#include "neural_network/components/solver.h"
#include "neural_network/neural.h"

char solver_check_valid(NeuralNetworkSolver *solver){
    return solver != NULL;
}

char check_matrix_list(List *m, int expected_n, int expected_m){
    if(m == NULL || listGetSize(m) == 0) return 0;
    
    const int list_size = listGetSize(m);
    int i;
    Matrix *mat;
    for(i = 0; i < list_size; i++){
        mat = LIST_DER(Matrix *, listGet(m, i));
        if(mat == NULL || matrixGetN(mat) != expected_n || matrixGetM(mat) != expected_m) return 0;
    }
    
    return 1;
}

int neural_list_get_random(List *donor, List *dropoff){
    int index = rand() % listGetSize(donor);
    int num = LIST_DER(int, listRemoveRet(donor, index));
    listAppend(dropoff, &num);
    return num;    
}

void neural_layers_destroy(void *layers){
    Matrix **layers_list = (Matrix **) layers;
    int i;
    
    for(i = 0; i < 8; i++){
        if(layers_list[i] != NULL){
            matrixDestroy(layers_list[i], 0);
        }
    }
}

NeuralNetwork *neural_network_create(NeuralNetworkSolver *solver, char file_flag, char *filename, int seed){
    if(!solver_check_valid(solver) || (file_flag && filename == NULL)) return NULL;
    NeuralNetwork *network = (NeuralNetwork *) calloc(1, sizeof(NeuralNetwork));
    assert(network != NULL);
    srandom(seed);
    
    neural_network_set_solver(solver, network);
    if(!solver_check_valid(network->solver)){
        printf("Error in inserting solver. Destroying network.\n");
        neural_network_destroy(network);
        return NULL;
    }
    /*activ_fun_set_fun(network, activation_function_flag);
    if(network->activation_function == NULL || network->d_activation_function == NULL){
        printf("Error in setting activation functions. Destroying network.\n");
        neural_network_destroy(network);
        return NULL;
    }*/
    
    network->log = file_flag;
    if(file_flag){
        network->logFile = fopen(filename, "w");
        if(network->logFile == NULL){
            printf("Unable to make log file. Not Logging.\n");
            network->log = 0;
        }
    }
    
    return network;
}
void neural_network_destroy(NeuralNetwork *network){
    if(network == NULL) return;
    
    if(network->logFile != NULL){
        fclose(network->logFile);
    }
    
    if(solver_check_valid(network->solver)){
        //TODO:Create destroyer for this later
    }
    
    free(network);
}

char neural_network_add_input_layer(NeuralNetwork *network, int size){
    if(size < 1 || network == NULL || solver_get_num_layers(network->solver) > 0) return 0; //Need this to be the first so size has to be 0
    
    network->solver->add_input_layer(network->solver, size);
    
    return 1; //Maybe add checks for solver addition
}

char neural_network_add_hidden_layer(NeuralNetwork *network, int size, int activation_function_flag){
    if(size < 1 || network == NULL || solver_get_num_layers(network->solver) < 1) return 0; //Cannot be first so size must not be 0
    
    network->solver->add_hidden_layer(network->solver, size, activation_function_flag);
    
    return 1; //Maybe add checks for solver addition
}

char neural_network_add_output_layer(NeuralNetwork *network, int size){
    return neural_network_add_hidden_layer(network, size, ACTIV_FUNC_SIGMOID); //Did this for now.. If anything results in being different on the output layer, I'll add it.
}

void neural_network_train(NeuralNetwork *network, List *x, List *y){
    //Initial checks
    if(network == NULL
        || !solver_check_valid(network->solver)
        || solver_get_num_layers(network->solver) <= 1
        || !check_matrix_list(x, solver_get_layer_n_val(network->solver, 0), 1)
        || !check_matrix_list(y, solver_get_layer_n_val(network->solver, solver_get_num_layers(network->solver) - 1), 1)
        || listGetSize(x) != listGetSize(y)
    ) return;
    
    //Get some values that get reused often
    const int list_size = listGetSize(x); //Constant that will most likely be used continuously.
    Matrix *output_layer = solver_get_output_layer(network->solver);    //LIST_DER(Matrix *, listGet(network->a, listGetSize(network->a) - 1));
    
    //Create a pile of indexes such that I can select randomly the order of inputs in each round.
    //This allows me to not have to shuffle the original List x or y, just grab the input.
    List *todo_index = listCreate(listGetSize(x), sizeof(int), NULL, NULL);
    assert(todo_index != NULL);
    List *discard_index = listCreate(listGetSize(x), sizeof(int), NULL, NULL);
    assert(discard_index != NULL);
    
    //Initialize the todo_index, filling it with listGetSize(x) indexes from [0, listGetSize(x) - 1]
    int i;
    int j = 100;
    for(i = 0; i < list_size; i++){
        listAppend(todo_index, &i); //Remember it needs a pointer to the object so it needs to be the address of the number
    }
    
    if(network->log) fprintf(network->logFile, "{\"Iterations\":{");
    
    //TODO:Add max iterations and alpha checking later, defaulting to 100 rounds for now
    do{
        printf("Rounds remaining: %d\n", j);
        if(network->log) fprintf(network->logFile, "\"Iteration %d\":{\"Results\":{", 100-j);
        for(i = 0; i < list_size; i++){
            //Get a random index to get a specific (random) entry
            int sel_ind = neural_list_get_random(todo_index, discard_index);
            Matrix *x_mat = LIST_DER(Matrix *, listGet(x, sel_ind));
            Matrix *y_mat = LIST_DER(Matrix *, listGet(y, sel_ind));
            
            if(network->log){
                fprintf(network->logFile, "\"Set %d\":{\"Input\":[", sel_ind);
                matrixPrintJSON(x_mat, network->logFile);
            }
            
            //ForwardPropagate it
            network->solver->forwardPropagate(network->solver, x_mat);
            
            //TODO: Get the error
            
            //Log it
            if(network->log){
                fprintf(network->logFile, "],\"Output\":[");
                matrixPrintJSON(output_layer, network->logFile);
                fprintf(network->logFile, "],\"Expected\":[");
                matrixPrintJSON(y_mat, network->logFile);
                fprintf(network->logFile, "]}");
                //If I have more entries to go through this round, add a comma
                if(i < list_size - 1){
                    fprintf(network->logFile, ",");
                }
            }
            
            //BackPropagate it
            network->solver->backPropagate(network->solver, x_mat, y_mat);
        }
        
        //Reset the todo_index and discard_index
        //I'm assuming todo_index is originally empty and discard_index is full
        List *temp = discard_index;
        discard_index = todo_index;
        todo_index = temp;
        
        j--;
        //TODO fix logging comma after while loop halt has been setMat
        if(network->log){
            fprintf(network->logFile, "}}"); //potential to add error here
            if(j) fprintf(network->logFile, ",");
        }
    }while(j); //Each loop here consists of one round
    if(network->log) fprintf(network->logFile, "}}");
    
    listDestroy(todo_index);
    listDestroy(discard_index);
}

List *neural_network_classify(NeuralNetwork *network, List *input){
    //TODO all of it...
}

// void forwardPropagate(NeuralNetwork *network, Matrix *input);
// void backPropagate(NeuralNetwork *network, Matrix *input, Matrix *y);
// void update(NeuralNetwork *network);

// void desMatWrap(void *matrix){
//     if(matrix == NULL) return;
// 
//     matrixDestroy(LIST_DER(Matrix *, matrix), 0);
// }
// 
// List *matrixify(List *entries);
// List *classify(List *cls, int bits);
// int outToInt(Matrix *output);
// double getError(Matrix *a, Matrix *y);
// void sigmoid(Matrix *matrix);
// double invSigmoid(Matrix *vector);
// double randDouble(double low, double high);
// 
// int outToInt(Matrix *output){
//     if(output == NULL) return -1;
//     
//     matrixResetIter(output);
//     
//     int i = 0, val = 0;
//     double value;
//     
//     while(!isnan(value = matrixGetNext(output, -1))){
//         val += 1 << (i++);
//     }
//     
//     return val;
// }
// 
// void sigmoid(Matrix *matrix){
//     if(matrix == NULL) return;
//     
//     matrixResetIter(matrix);
//     
//     double value;
//     
//     while(!isnan(value = matrixGetNext(matrix, -1))){
//         value = 1.0 / (1.0 + exp(-value));
//         if(isnan(value)){
//             printf("bad nums\n");
//             value = 0;
//         }
//         matrixSetPrevious(matrix, value);
//     }
// }
// 
// double invSigmoid(Matrix *vector){
//     if(vector == NULL) return NAN;
//     
//     double sum = 0, value;
//     
//     matrixResetIter(vector);
//     
//     while(!isnan(value = matrixGetNext(vector, -1))){
//         sum += value * (1 - value);
//     }
//     return sum;
// }
// 
// double randDouble(double low, double high){
//     double num = (double) rand() / RAND_MAX;
//     return low + num * (high - low);
// }
// 
// void forwardPropagate(NeuralNetwork *network, Matrix *input){
//     if(input == NULL || network == NULL) return;
// 
//     Matrix **a = network->a->data;
//     Matrix **finalA = a + network->a->size;
//     Matrix **b = network->b->data;
//     Matrix **w = network->w->data;
// 
//     matMul(*w, input, *a, 0);
//     matAdd(*b, *a, *a, 0);
//     sigmoid(*a);
//     a++;
//     w++;
//     b++;
//     
//     while(a != finalA){
//         matMul(*w, *(a - 1), *a, 0);
//         matAdd(*b, *a, *a, 0);
//         sigmoid(*a);
// 
//         a++;
//         b++;
//         w++;
//     }      
// }
// 
// void backPropagate(NeuralNetwork *network, Matrix *input, Matrix *y){
//     if(network == NULL || input == NULL) return;
// 
//     Matrix **a = (Matrix **) get(network->a, network->a->size - 1);
//     Matrix **dA = (Matrix **) get(network->dA, network->dA->size - 1);
//     Matrix **aStop = (Matrix **) get(network->a, 0);
//     Matrix **w = (Matrix **) get(network->w, network->w->size - 1);
//     Matrix **b = (Matrix **) get(network->b, network->b->size - 1);
//     //Matrix **dW = (Matrix **) get(network->dW, network->dW->size - 1);
// 
//     //dC/dz constant
//     //double sigPrime = invSigmoid(*a);
//     //dC/da
//     matSub(*a, y, *dA, 0);
//     //constantMul(*a, 2);
//     
//     while(a > aStop){
//         //dC/dZ
//         constantMul(*dA, invSigmoid(*a));
//         //dC/dB
//         //dC/dW
//         //sigPrime = invSigmoid(*a);
//         //dC/da
//         matMul(*w, *dA, *(dA - 1), 1);
//         constantMul(*dA, network->lRate);
//         matMul(*dA, *(a - 1), *w, 18);
//         matSub(*b, *dA, *b, 0);
//         b--;
//         dA--;
//         w--;
//         a--;
//     }
// 
//     constantMul(*dA, invSigmoid(*a));
//     constantMul(*dA, network->lRate);
// 
//     matSub(*b, *dA, *b, 0);
// 
//     matMul(*dA, input, *w, 18);
//     
// }
// 
// double getError(Matrix *a, Matrix *y){
//     if(a == NULL || y == NULL) return -1;
// 
//     Matrix temp;
//     double tempList[a->n * a->m];
//     temp.mat = tempList;
//     temp.n = a->n;
//     temp.m = a->m;
// 
//     matSub(a, y, &temp, 0);
//     return dotProd(temp.mat, 1, temp.mat, 1, temp.mat + (a->m * a->n));
// }
// 
// List *matrixify(List *entries){
//     if(entries == NULL) return NULL;
// 
//     List *matrices = createDataList(entries->size, sizeof(Matrix *), NULL, desMatWrap);
//     
//     if(matrices == NULL){
//         printf("Insufficient space to matrixify. Exiting.\n");
//         exit(0);
//     }
// 
//     int i;
//     for(i = 0; i < entries->size; i++){
//         List *entry = *((List **)get(entries, i));
// 
//         Matrix *m = createMatrix(entry->size, 1, NULL, entry->size);
//         if(m == NULL){
//             printf("Error in matrixify. Exiting. \n");
//             exit(0);
//         }
// 
//         int j;
//         for(j = 0; j < entry->size; j++){
//             *(m->mat + j) = *((double *) get(entry, j));
//         }
// 
//         append(matrices, &m);
//     }
// 
//     return matrices;
// }
// 
// List *classify(List *cls, int bits){
//     if(cls == NULL) return NULL;
// 
//     List *classes = createDataList(cls->size, sizeof(Matrix *), NULL, desMatWrap);
// 
//     if(classes == NULL){
//         printf("Insufficient space to makes classes. Exiting.\n");
//         exit(0);
//     }
// 
//     int *clsList = (int *) cls->data;
//     int *clsListEnd = clsList + cls->size;
//     int *i = clsList;
// 
//     int max = 0;
//     while(i < clsListEnd){
//         if(*i > max) max = *i;
//         i++;
//     }
// 
//     if((int)ceil(log10(max + 1)/log10(2)) > bits){
//         printf("Insufficient number of nodes on output layer for given classes. Rebuild a new network.\n");
//         return NULL;
//     }
// 
//     for(i = clsList; i < clsListEnd; i++){
//         int num = *i;
// 
//         Matrix *m = createMatrix(bits, 1, NULL, bits);
//         if(m == NULL){
//             printf("Insufficient memory to produce a Matrix in neural.c/classify(). Exiting.\n");
//             exit(0);
//         }
// 
//         double *list = m->mat;
//         while(num){
//             double val = num & 1;
//             *list = val;
//             num >>= 1;
//             list++;
//         }
// 
//         append(classes, &m);
//     }
//     return classes;
// }
// 
// void update(NeuralNetwork *network){
//     if(network == NULL) return;
//     Matrix **wStop = (Matrix **) get(network->w, network->w->size - 1);
//     Matrix **w = (Matrix **) get(network->w, 0);
//     Matrix **dW = (Matrix **) get(network->dW, 0);
//     Matrix **b = (Matrix **) get(network->b, 0);
//     Matrix **dB = (Matrix **) get(network->dB, 0);
// 
// 
//     while(w <= wStop){
//         constantMul(*dW, network->lRate);
//         constantMul(*dB, network->lRate);
//         matSub(*w, *dW, *w, 0);
//         setMat(*dW, 0);
//         matSub(*b, *dB, *b, 0);
//         setMat(*dB, 0);
//         dW++;
//         w++;
//         dB++;
//         b++;
//     }
// }
// 
// List *classifyNetwork(NeuralNetwork *network, Data *data){
//     List *x = matrixify(data->feats);
//     Matrix *aL = *((Matrix **) get(network->a, network->a->size - 1));
//     List *y = createDataList(data->cls->size, sizeof(int), NULL, NULL);
// 
//     Matrix **xList = (Matrix **) get(x, 0);
//     Matrix **xStop = (Matrix **) get(x, x->size - 1);
// 
//     while(xList <= xStop){
//         forwardPropagate(network, *xList);
//         int val = outToInt(aL);
// 
//         append(y, &val);
//         xList++;
//     }
//     deleteDataList(x);
//     return y;
// }
// 
// void trainNetwork(NeuralNetwork *network, Data *data){
//     List *x = matrixify(data->feats);
//     int outputNodes = *((int *) get(network->layers, network->layers->size - 1));
//     List *y = classify(data->cls, outputNodes);
//     Matrix *aL = *((Matrix **) get(network->a, network->a->size - 1));
//     
//     int j = 0;
//     int tally = x->size;
//     int tally2 = x->size;
//     const int goal = (int) (network->alpha * x->size);
//     if(network->log) fprintf(network->logFile, "{\"Iterations\":{");
//     while((tally > goal || tally2 > goal) && j < network->maxIter){
//         if(network->log && j){
//             fprintf(network->logFile, ",");
//         }
//         int checkingPass = 0;
//         if(tally <= goal){
//             checkingPass = 1;
//         }
// 
//         tally2 = tally;
//         tally = 0;
//         int i = 0;
//         Matrix **yList = (Matrix **) y->data;
//         Matrix **xList = (Matrix **) x->data;
//         Matrix **xStop = (Matrix **) get(x, x->size - 1);
//         if(network->log) fprintf(network->logFile, "\"Iteration %d\":{\"Results\":{", j + 1);
//         while(xList <= xStop){
//             i++;
//             if(network->log){
//                 fprintf(network->logFile, "\"Set %d\":{\"Input\":[", i);
//                 printMatrixJSON(*xList, network->logFile);
//             }
// 
//             forwardPropagate(network, *xList);
//             double curErr = getError(aL, *(yList)) / 2;
// 
//             if(network->log){
//                 fprintf(network->logFile, "],\"Output\":[");
//                 printMatrixJSON(aL, network->logFile);
//                 fprintf(network->logFile, "],\"Expected\":[");
//                 printMatrixJSON(*yList, network->logFile);
//                 fprintf(network->logFile, "]}");
//                 if(xList != xStop){
//                     fprintf(network->logFile, ",");
//                 }
//             }
// 
//             //error += curErr;
//             if(curErr >= .005){
//                 if(!checkingPass){
//                     backPropagate(network, *xList, *yList);
//                     //update(network);
//                 }
//                 if(!tally){
//                     printf("\tMisses at %d", i);
//                 }else{
//                     printf(",%d", i);
//                 }
//                 tally++;
//             }
//             xList++;
//             yList++;
//         }
//         printf("\n");
//         double error = (double) tally / (double) x->size;
//         if(network->log) fprintf(network->logFile, "},\"Error\": %.10lf}", error);
//         printf("Iteration %d. Error: %lf\n", j + 1, error);
//         if(tally > goal) j++;
//     }
//     if(network->log) fprintf(network->logFile, "}}");
//     deleteDataList(x);
//     deleteDataList(y);
// 
// }
// 
// void destroyNN(NeuralNetwork *network){
//     if(network == NULL) return;
// 
//     deleteDataList(network->a);
//     deleteDataList(network->dA);
//     deleteDataList(network->w);
//     deleteDataList(network->dW);
//     deleteDataList(network->b);
//     deleteDataList(network->dB);
//     deleteDataList(network->layers);
// 
//     if(network->logFile != NULL){
//         fclose(network->logFile);
//     }
// 
//     free(network);
// }
// 
// NeuralNetwork *createNN(int *layers, int numLayers, double alpha, int maxIter, double lRate, char log, char *filename, int seed){
//     if(numLayers < 2){
//         return NULL;
//     }else{
//         int i = 0;
//         for(;i < numLayers; i++){
//             if(layers[i] <= 0){
//                 printf("Problem with layers...");
//                 return NULL;
//             }
//         }
//     }
//     srandom(seed);
// 
//     NeuralNetwork *network = (NeuralNetwork *) calloc(1, sizeof(NeuralNetwork));
// 
//     if(network == NULL){
//         printf("Insufficient space to make network. Exiting.\n");
//         exit(0);
//     }
//     //Create a, b, dA, and dB
//     network->a = createDataList(numLayers - 1, sizeof(Matrix *), NULL, desMatWrap);
//     if(network->a == NULL){
//         printf("Insufficient space to make network nodes. Exiting.\n");
//         exit(0);
//     }
//     network->dA = createDataList(numLayers - 1, sizeof(Matrix *), NULL, desMatWrap);
//     if(network->dA == NULL){
//         printf("Insufficient space to make network nodes differentials. Exiting.\n");
//         exit(0);
//     }
//     network->b = createDataList(numLayers - 1, sizeof(Matrix *), NULL, desMatWrap);
//     if(network->b == NULL){
//         printf("Insufficient space to make network bs. Exiting.\n");
//         exit(0);
//     }
//     network->dB = createDataList(numLayers - 1, sizeof(Matrix *), NULL, desMatWrap);
//     if(network->dB == NULL){
//         printf("Insufficient space to make network b differentials. Exiting.\n");
//         exit(0);
//     }
//     
//     //Create w, dW
//     network->w = createDataList(numLayers - 1, sizeof(Matrix *), NULL, desMatWrap);
//     if(network->w == NULL){
//         printf("Insufficient space to make network weights. Exiting.\n");
//         exit(0);
//     }
//     network->dW = createDataList(numLayers - 1, sizeof(Matrix *), NULL, desMatWrap);
//     if(network->dW == NULL){
//         printf("Insufficient space to make network weight differentials. Exiting.\n");
//         exit(0);
//     }
//     network->layers = createDataList(numLayers, sizeof(int), NULL, NULL);
// 
//     List *lists[6] = {network->a, network->dA, network->b, network->dB, network->w, network->dW};
// 
//     //Creating all the lists
//     int *pLayer = layers, *cLayer = layers + 1;
//     int nCount, wCount;
//     append(network->layers, pLayer);
//     while(cLayer < layers+numLayers){
//         nCount = *cLayer;
//         wCount = *pLayer;
//         int i;
//         append(network->layers, &nCount);
// 
//         for(i = 0;i < 4;i++){
//             Matrix *m = createMatrix(nCount, 1, NULL, nCount);
//             if(m == NULL){
//                 printf("Insufficient space to make network list. Exiting.\n");
//                 exit(0);
//             }
//             append(*(lists+i), &m);
//         }
// 
//         for(;i<6;i++){
//             Matrix *m = createMatrix(nCount, wCount, NULL, wCount * nCount);
//             if(m == NULL){
//                 printf("Insufficient space to make network list. Exiting.\n");
//                 exit(0);
//             }
//             if(i == 4){
//                 double *l = m->mat;
//                 int len = m->m * m->n;
//                 int j;
//                 for(j = 0; j < len; j++){
//                     *(l++) = randDouble(-1.0, 1.0);
//                 }
//             }
// 
//             append(*(lists+i), &m);
//         }
// 
//         pLayer = cLayer;
//         cLayer++;
//     }
//     network->alpha = alpha;
//     if(maxIter < 1){
//         network->maxIter = 10;
//     }else{
//         network->maxIter = maxIter;
//     }
// 
//     network->lRate = lRate;
//     if(log && filename != NULL){
//         network->logFile = fopen(filename, "w");
//         if(network->logFile != NULL) network->log = log;
//         else{
//             printf("Unable to make log file. Not logging.\n");
//             network->log = 0;
//         }
//     }else{
//         network->log = 0;
//         network->logFile = NULL;
//     }
// 
//     return network;
// }
