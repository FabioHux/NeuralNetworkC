#ifndef _LIBRARY_MODEL_CONST_
#define _LIBRARY_MODEL_CONST_
#include <stdio.h>
#include <stdlib.h>

/**
 * List.c functions
 **/

//Definitions
#define LIST_DER(TYPE, X) *((TYPE *) X)

//Opaque Struct
typedef struct _list List;

//List Creator/Destroyer
List *listCreate(int len, size_t size, int (*cmp)(void *, void *), void (*destroy)(void *));
void listDestroy(List *list);

//Instance Functions
char listAppend(List *list, void *value);
char listInsert(List *list, void *value, int index);
char listInsertSorted(List *list, void *value);
void *listRemoveRet(List *list, int index);

void *listGet(List *list, int index);
char listSet(List *list, int index, void *value);
int listGetSize(List *list);
int listGetESize(List *list);

int listIndexOf(List *list, void *value);
int listCmpVal(void *a, void *b, List *list); 

//Basic Comparator Functions
extern int list_int_cmp(void *a, void *b);
extern int list_double_cmp(void *a, void *b);

//Basic Destroyer Functions
void listDestroyer(void *element);
void nulled_matrix_destroy(void *matrix);


/**
 * Matrix.c functions
 **/

//Definitions
#define MATRIX_A_TRANS 1    /*Operation has the first Matrix Transposed*/
#define MATRIX_B_TRANS 2    /*Operation has the second Matrix Transposed*/
#define MATRIX_C_TRANS 4    /*Operation has the resulting Matrix Transposed*/
#define MATRIX_RESULT_ADD 8 /*Operation modified to have the result added to C*/
#define MATRIX_RESULT_SUB 16    /*Operation modified to have the result subtracted from C*/

//Opaque Struct
typedef struct _matrix Matrix;

//Matrix Creator/Destroyer
Matrix *matrixCreate(int n, int m, double *values, int valuesLen);
double *matrixDestroy(Matrix *matrix, char flags);

//Instance Functions
Matrix *matrixMul(Matrix *a, Matrix *b, Matrix *c, char flags);
Matrix *matrixAdd(Matrix *a, Matrix *b, Matrix *c, char flags);
Matrix *matrixSub(Matrix *a, Matrix *b, Matrix *c, char flags);
void matrixConstantAdd(Matrix *a, double c);
void matrixConstantMul(Matrix *a, double c);

double dotProd(double *a, long stepA, double *b, long stepB, double *aStop);

int matrixGetM(Matrix *a);
int matrixGetN(Matrix *a);
double matrixGetValue(Matrix *a, int n, int m);
double matrixGetNext(Matrix *a, int index);

void matrixSetMat(Matrix *a, double num);
void matrixSetValue(Matrix *a, int n, int m, double value);
void matrixSetPrevious(Matrix *a, double value);
void matrixResetIter(Matrix *a);

void matrixPrint(Matrix *a);
void matrixPrintJSON(Matrix *a, FILE *output);


/**
 * Neural Solver functions
 **/

//Opaque Struct
typedef struct _neural_network_solver NeuralNetworkSolver;

NeuralNetworkSolver *neural_network_solver_sgd(double alpha, double rate);
//Add more when created like Momentum or RMSProp?

/**
 * Neural.c functions
 **/

//Definitions
#define ACTIV_FUNC_SIGMOID 0
#define ACTIV_FUNC_RELU 1
//Add more when created

//Opaque Struct
typedef struct _neural_network NeuralNetwork;

//Neural Network Creator/Destroyer
NeuralNetwork *neural_network_create(NeuralNetworkSolver *solver, char file_flag, char *filename, int seed);
void neural_network_destroy(NeuralNetwork *network);

//Instance Functions
char neural_network_add_input_layer(NeuralNetwork *network, int size);
char neural_network_add_hidden_layer(NeuralNetwork *network, int size, int activation_function_flag);
char neural_network_add_output_layer(NeuralNetwork *network, int size);

void neural_network_train(NeuralNetwork *network, List *x, List *y);
List *neural_network_classify(NeuralNetwork *network, List *input);

#endif
