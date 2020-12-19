#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrix.h"

struct _matrix{
    double *mat, *matEnd, *matIter;
    int n, m;
};

//Private helper functions
void addOp(double *target, double value){
    *target += value;
}

void subOp(double *target, double value){
    *target -= value;
}

void setOp(double *target, double value){
    *target = value;
}

int matrixGetM(Matrix *a){
    if(a == NULL)
        return -1;
    return a->m;
}

int matrixGetN(Matrix *a){
    if(a == NULL)
        return -1;
    return a->n;
}
double matrixGetValue(Matrix *a, int n, int m){
    if(a == NULL || n < 0 || m < 0 || n >= a->n || m >= a->m)
        return NAN;
    return *(a->mat + (n * a->m) + m);
}

void matrixSetValue(Matrix *a, int n, int m, double value){
    if(a != NULL && !isnan(value) && n >= 0 && m >= 0 && n < a->n && m < a->m)
        *(a->mat + (n * a->m) + m) = value;
}

double matrixGetNext(Matrix *a, int index){
    if(a == NULL) return NAN;
    
    if(index == -1){
        if(a->matIter != a->matEnd){
            return *(a->matIter++);
        }else{
            a->matIter = a->mat;
        }
    }else if(index >= 0 && index < a->n * a->m){
        a->matIter = a->mat + index;
        return *(a->matIter++);
    }
    
    return NAN;
}

void matrixResetIter(Matrix *a){
    if(a != NULL) a->matIter = a->mat;
}


void matrixSetPrevious(Matrix *a, double value){    
    if(a != NULL && !isnan(value) && a->matIter > a->mat && a->matIter <= a->matEnd){
        *(a->matIter - 1) = value;
    }
}




Matrix *matrixMul(Matrix *a, Matrix *b, Matrix *c, char flags){
    if(a == NULL || b == NULL || c == NULL) return NULL;
    
    int aLStep = a->m, aDStep = 1, bLStep = 1, bDStep = b->m, cLStep = 1, cRStep = 0;
    double *aMat = a->mat, *bMat = b->mat, *cMat = c->mat, *aStop = a->matEnd, *bStop = b->matEnd, *aHalt = aMat + aLStep;
    void (*op)(double *, double) = setOp;
    
    if(flags & 4){
        cLStep = c->m;
        cRStep = (cLStep * c->n) - 1;
        if((flags & 1 && aLStep != cLStep) || (!(flags & 1) && a->n != cLStep) ||(flags & 2 && b->n != c->n) || (!(flags & 2) && bDStep != c->n)){
            return NULL;
        }
    }else if((flags & 1 && aLStep != c->n) || (!(flags & 1) && a->n != c->n) ||(flags & 2 && b->n != c->m) || (!(flags & 2) && bDStep != c->m)){
        return NULL;
    }
    
    if(flags & 1){
        aDStep = aLStep;
        aLStep = 1;
        aHalt = aStop;
        aStop = aMat + aDStep;
    }
    
    if(flags & 2){
        bLStep = bDStep;
        bDStep = 1;
        bStop = bMat + b->n;
    }
    
    if(flags & 8){
        if(flags & 24){
            return NULL;
        }
        op = addOp;
    }else if(flags & 16){
        op = subOp;
    }
    
    for(;aMat < aStop; aMat += aLStep, aHalt += aLStep){
        for(bMat = b->mat; bMat < bStop; bMat += bLStep){
            op(cMat, dotProd(aMat, aDStep, bMat, bDStep, aHalt));
            cMat += cLStep;
        }
        cMat -= cRStep;
    }
    
    return c;
}

Matrix *matrixAdd(Matrix *a, Matrix *b, Matrix *c, char flags){
    if(a == NULL || b == NULL || c == NULL) return NULL;
    int aStep = 1, bStep = 1, cStep = 1, aOff = 0, bOff = 0, cOff = 0, n = c->n, m = c->m;
    double *aMat = a->mat, *bMat = b->mat, *cMat = c->mat;

    if(flags & 4){
        cStep = c->m;
        cOff = (c->m * c->n) - 1;
        n = c->m;
        m = c->n;
    }

    if(flags & 1){
        if(a->m != n || a->n != m){
            printf("I dipped matAdd.\n");
            return NULL;
        }
        aStep = a->m;
        aOff = (a->m * a->n) - 1;
    }else if(a->n != n || a->m != m){
        printf("I dipped2 matAdd.\n");
        return NULL;
    }

    if(flags & 2){
        if(b->m != n || b->n != m){
            printf("I dipped3 matAdd.\n");
            return NULL;
        }
        bStep = b->m;
        bOff = (b->m * b->n) - 1;
    }else if(b->n != n || b->m != m){
        printf("I dipped4 matAdd.\n");
        return NULL;
    }

    int i,j;
    
    for(i = 0; i < n; i++, aMat -= aOff, bMat -= bOff, cMat -= cOff){
        for(j = 0; j < m; j++, aMat += aStep, bMat += bStep, cMat += cStep){
            *cMat = *bMat + *aMat;
        }
    }
    return c;
}


Matrix *matrixSub(Matrix *a, Matrix *b, Matrix *c, char flags){
    if(a == NULL || b == NULL || c == NULL) return NULL;
    int aStep = 1, bStep = 1, cStep = 1, aOff = 0, bOff = 0, cOff = 0, n = c->n, m = c->m;
    double *aMat = a->mat, *bMat = b->mat, *cMat = c->mat;

    if(flags & 4){
        cStep = c->m;
        cOff = (c->m * c->n) - 1;
        n = c->m;
        m = c->n;
    }

    if(flags & 1){
        if(a->m != n || a->n != m){
            printf("I dipped matAdd.\n");
            return NULL;
        }
        aStep = a->m;
        aOff = (a->m * a->n) - 1;
    }else if(a->n != n || a->m != m){
        printf("I dipped2 matAdd.\n");
        return NULL;
    }

    if(flags & 2){
        if(b->m != n || b->n != m){
            printf("I dipped3 matAdd.\n");
            return NULL;
        }
        bStep = b->m;
        bOff = (b->m * b->n) - 1;
    }else if(b->n != n || b->m != m){
        printf("I dipped4 matAdd.\n");
        return NULL;
    }

    int i,j;
    
    for(i = 0; i < n; i++, aMat -= aOff, bMat -= bOff, cMat -= cOff){
        for(j = 0; j < m; j++, aMat += aStep, bMat += bStep, cMat += cStep){
            *cMat = *aMat - *bMat;
        }
    }
    return c;
}

Matrix *matrixCreate(int n, int m, double *values, int valuesLen){
    if(n * m != valuesLen || !n || !m) return NULL;

    Matrix *matrix = calloc(1, sizeof(Matrix));
 
    if(matrix == NULL){
        printf("Error making matrix. Insufficient space. Exiting.\n");
        exit(0);
    }

    if(values == NULL){
        matrix->mat = (double *) calloc(valuesLen, sizeof(double));

        if(matrix->mat == NULL){
            printf("Error making matrix list. Insufficient space. Exiting.\n");
            exit(0);
        }
    }else{
        matrix->mat = values;
    }

    matrix->n = n;
    matrix->m = m;
    matrix->matIter = matrix->mat;
    matrix->matEnd = matrix->mat + valuesLen;
    return matrix;
}

double *matrixDestroy(Matrix *matrix, char flags){
    if(matrix == NULL) return NULL;
    double *list = NULL;

    if(flags & 1){
        list = matrix->mat;
    }else{
        free(matrix->mat);
    }

    free(matrix);

    return list;
}

double dotProd(double *a, long stepA, double *b, long stepB, double *aStop){
    double sum = 0;

    asm volatile(
        "pxor %%xmm0, %%xmm0\n\t"
        "jmp 2f\n\t"
        "1: movsd (%1), %%xmm1\n\t"
        "movsd (%3), %%xmm2\n\t"
        "mulsd %%xmm1, %%xmm2\n\t"
        "addsd %%xmm2, %%xmm0\n\t"
        "addq %2, %1\n\t"
        "addq %4, %3\n\t"
        "2: cmpq %1, %5\n\t"
        "jg 1b\n\t"
        "movq %%xmm0, %0\n\t"
        : "=r" (sum)
        : "D" (a), "S" (stepA * 8), "d" (b), "r" (stepB * 8), "r" (aStop)
        :"%xmm0", "%xmm1", "%xmm2"
    );

    /*while(a < aStop){
        sum += *a * *b;
        a += stepA;
        b += stepB;
    }*/

    return sum;
}

void matrixPrint(Matrix *a){
    if(a == NULL) return;

    int i,j;
    for(i = 0; i < a->n; i++){
        if(a->n <= 20 || (i < 5 || i >= a->n - 5)){
            for(j = 0; j < a->m; j++){
                if(a->m <= 20 || (j < 5 || j >= a->m - 5))
                    printf("%10.2lf\t", a->mat[(i * a->m) + j]);
                else if(j == 5){
                    int k = 3;
                    for(;k > 0; k--){
                        printf("%*s.\t", 10, "");
                    }
                    j = a->m - 6;
                }
            }
            printf("\n");
        }else if(i == 5){
            if(a->m <= 20){
                j = a->m;
            }else{
                j = 13;
            }
            int k = 3;
            for(;k > 0; k--){
                int l;
                for(l = 0;l < j; l++){
                    printf("%*s.\t",10, "");
                }
                printf("\n");
            }
            i = a->n - 6;
        }
    }
}

void matrixPrintJSON(Matrix *a, FILE *output){
    if(a == NULL || output == NULL) return;

    int i,j;
    /*for(i = 0; i < a->n; i++){
        //if(a->n > 20 && (i < 5 || i >= a->n - 5)){
            if(i) fprintf(output, ",");
            fprintf(output, "[");
            for(j = 0; j < a->m - 1; j++){
                //if(a->m > 20 && (j < 5 || j >= a->m - 5))
                    fprintf(output, "%10.2lf,", a->mat[(i * a->m) + j]);
            }
            fprintf(output, "%10.2lf]", a->mat[((i + 1) * a->m) - 1]);
        //}
    }*/

    
    for(i = 0; i < a->n; i++){
        if(i) fprintf(output, ",");

        if(a->n <= 20 || (i < 5 || i >= a->n - 5)){
            fprintf(output, "[");
            for(j = 0; j < a->m - 1; j++){
                if(a->m <= 20 || (j < 5 || j >= a->m - 5))
                    fprintf(output, "%.2lf,", a->mat[(i * a->m) + j]);
                else if(j == 5){
                    int k = 3;
                    for(;k > 0; k--){
                        fprintf(output, "\".\",");
                    }
                    j = a->m - 6;
                }
            }
            fprintf(output, "%.2lf]", a->mat[((i + 1) * a->m) - 1]);
        }else if(i == 5){
            if(a->m <= 20){
                j = a->m;
            }else{
                j = 13;
            }
            int k = 3;
            for(;k > 0; k--){
                int l;
                if(k < 3) fprintf(output, ",");
                fprintf(output, "[");
                for(l = 0;l < j-1; l++){
                    fprintf(output, "\".\",");
                }
                fprintf(output, "\".\"]");
            }

            i = a->n - 6;
        }
    }
}

void matrixConstantAdd(Matrix *a, double c){
    double *val = a->mat;
    double *stop = val + (a->m * a->n);

    while(val < stop){
        *(val++) += c;
    }
}

void matrixConstantMul(Matrix *a, double c){
    double *val = a->mat;
    double *stop = val + (a->m * a->n);

    while(val < stop){
        *(val++) *= c;
    }
}

void matrixSetMat(Matrix *a, double num){
    double *val = a->mat;
    double *stop = val + (a->m * a->n);

    while(val < stop){
        *(val++) = num;
    }
}
