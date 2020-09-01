#ifndef MATRIX_CONST
#define MATRIX_CONST

typedef struct{
    double *mat;
    int n, m;
} Matrix;

Matrix *matMul(Matrix *a, Matrix *b, Matrix *c, char flags);
Matrix *matAdd(Matrix *a, Matrix *b, Matrix *c, char flags);
Matrix *matSub(Matrix *a, Matrix *b, Matrix *c, char flags);
Matrix *createMatrix(int n, int m, double *values, int valuesLen);
double *destroyMatrix(Matrix *matrix, char flags);
double dotProd(double *a, long stepA, double *b, long stepB, double *aStop);
void printMatrix(Matrix *a);
void printMatrixJSON(Matrix *a, FILE *output);
void constantAdd(Matrix *a, double c);
void constantMul(Matrix *a, double c);
void setMat(Matrix *a, double num);

#endif