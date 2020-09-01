#ifndef NEURAL_CONST
#define NEURAL_CONST
#include <stdio.h>
#include "data.h"

typedef struct{
    List *a, *dA, *b, *dB, *w, *dW, *layers;
    FILE *logFile;
    double alpha, lRate;
    int maxIter;
    char log;
} NeuralNetwork;


NeuralNetwork *createNN(int *layers, int numLayers, double alpha, int maxIter, double lRate, char log, char *filename, int seed);
void destroyNN(NeuralNetwork *network);
void trainNetwork(NeuralNetwork *network, Data *data);
List *classifyNetwork(NeuralNetwork *network, Data *data);
#endif