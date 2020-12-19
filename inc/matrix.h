#ifndef MATRIX_CONST
#define MATRIX_CONST

typedef struct _matrix Matrix;

Matrix *matrixMul(Matrix *a, Matrix *b, Matrix *c, char flags);
Matrix *matrixAdd(Matrix *a, Matrix *b, Matrix *c, char flags);
Matrix *matrixSub(Matrix *a, Matrix *b, Matrix *c, char flags);
Matrix *matrixCreate(int n, int m, double *values, int valuesLen);
double *matrixDestroy(Matrix *matrix, char flags);
double dotProd(double *a, long stepA, double *b, long stepB, double *aStop);
void matrixPrint(Matrix *a);
void matrixPrintJSON(Matrix *a, FILE *output);
void matrixConstantAdd(Matrix *a, double c);
void matrixConstantMul(Matrix *a, double c);
void matrixSetMat(Matrix *a, double num);
int matrixGetM(Matrix *a);
int matrixGetN(Matrix *a);
double matrixGetValue(Matrix *a, int n, int m);
void matrixSetValue(Matrix *a, int n, int m, double value);
double matrixGetNext(Matrix *a, int index);
void matrixSetPrevious(Matrix *a, double value);
void matrixResetIter(Matrix *a);

#endif
