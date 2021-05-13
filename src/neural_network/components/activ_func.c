#include <math.h>
#include "libremodel.h"
#include "neural_network/components/activ_func.h"
#include "neural_network/neural.h"

void sigmoid(Matrix *input, Matrix *output){
    if(input == NULL || output == NULL) return;
    
    matrixResetIter(input);
    if(input != output){
        matrixResetIter(output);
    }
    double value;
    
    while(!isnan(value = matrixGetNext(input, -1))){
        if(input != output){
            matrixGetNext(output, -1);
        }
        
        value = 1.0 / (1.0 + exp(-value));
        if(isnan(value)){
            printf("bad nums\n");
            value = 0;
        }
        matrixSetPrevious(output, value);
    }
}

Matrix *der_sigmoid(Matrix *z, Matrix *a){
    if(z == NULL && a == NULL) return NULL;
    
    Matrix *vector = z;
    
    if(a != NULL) vector = a;
    
    Matrix *c = matrixCreate(matrixGetN(vector), matrixGetN(vector), NULL, matrixGetN(vector) * matrixGetN(vector));
    if(c == NULL) return NULL;
    
    double value, temp;
    int i = 0;
    
    matrixResetIter(vector);
    
    while(!isnan(value = matrixGetNext(vector, -1))){
//         expval = exp(value);
//         temp = expval + 1;
//         temp *= temp;
//         matrixSetValue(c, i, i, expval / temp);
        if(a == NULL) temp = 1.0 / (1.0 + exp(-value));
        else temp = value;
        matrixSetValue(c, i, i, temp * (1 - temp));
        i++;
    }
    return c;
}

void relu(Matrix *input, Matrix *output){
    if(input == NULL || output == NULL) return;
    
    matrixResetIter(input);
    if(input != output){
        matrixResetIter(output);
    }
    double value;
    
    while(!isnan(value = matrixGetNext(input, -1))){
        if(input != output){
            matrixGetNext(output, -1);
        }
        
        value = (value < 0) ? 0 : value;
        if(isnan(value)){
            printf("bad nums\n");
            value = 0;
        }
        matrixSetPrevious(output, value);
    }
}

Matrix *der_relu(Matrix *z, Matrix *a){
    if(z == NULL) return NULL;
    (void) a;
    
    Matrix *c = matrixCreate(matrixGetN(z), matrixGetN(z), NULL, matrixGetN(z) * matrixGetN(z));
    if(c == NULL) return NULL;
    
    double value;
    int i = 0;
    
    matrixResetIter(z);
    
    while(!isnan(value = matrixGetNext(z, -1))){
//         expval = exp(value);
//         temp = expval + 1;
//         temp *= temp;
//         matrixSetValue(c, i, i, expval / temp);
        matrixSetValue(c, i, i, (value < 0) ? 0 : 1);
        i++;
    }
    return c;
}

void activ_fun_set_fun(void (**activation_function)(Matrix *, Matrix *), Matrix *(**d_activation_function)(Matrix *, Matrix *), int flag){
    if(flag < 0 || activation_function == NULL || d_activation_function == NULL) return;
    
    switch(flag){
        case ACTIV_FUNC_SIGMOID:
            *activation_function = sigmoid;
            *d_activation_function = der_sigmoid;
            break;
        case ACTIV_FUNC_RELU:
            *activation_function = relu;
            *d_activation_function = der_relu;
            break;
        //While creating more functions, just make new flag values and add them to cases
    }
}
