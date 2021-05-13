#ifndef NEURAL_INNER_CONST
#define NEURAL_INNER_CONST
#include <stdio.h>
#include "libremodel.h"

struct _neural_network{
    FILE *logFile;
    NeuralNetworkSolver *solver;
    void (*activation_function)(Matrix *, Matrix *);
    Matrix *(*d_activation_function)(Matrix *, Matrix *);
    double (*loss_function)(Matrix *, Matrix *);
    void (*d_loss_function)(Matrix *, Matrix *, Matrix *);
    char log;
};

typedef struct _neural_network_hidden_solver NeuralNetworkHiddenSolver;

struct _neural_network_solver{
    NeuralNetworkHiddenSolver *hidden_solver;
    void (*add_input_layer)(NeuralNetworkSolver *, int);
    void (*add_hidden_layer)(NeuralNetworkSolver *, int, int);
    void (*add_output_layer)(NeuralNetworkSolver *, int);
    void (*backPropagate)(NeuralNetworkSolver *, Matrix *, Matrix *);
    void (*forwardPropagate)(NeuralNetworkSolver *, Matrix *);
};

char solver_check_valid(NeuralNetworkSolver *solver);

void forwardPropagate(NeuralNetwork *network, Matrix *input);
void backPropagate(NeuralNetwork *network, Matrix *input, Matrix *y);
void update(NeuralNetwork *network);

void neural_network_set_solver(NeuralNetworkSolver *solver, NeuralNetwork *network);

#endif
