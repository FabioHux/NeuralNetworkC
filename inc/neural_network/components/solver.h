#ifndef _NEURAL_SOLVER_CONST_
#define _NEURAL_SOLVER_CONST_
#include <assert.h>
#include "libremodel.h"
#include "neural_network/neural.h"


void neural_network_set_solver(NeuralNetworkSolver *solver, NeuralNetwork *network);

NeuralNetworkSolver *sgd_solver_create(double alpha, double rate);

char hidden_solver_check_valid(NeuralNetworkHiddenSolver *solver);


void generic_add_input_layer(NeuralNetworkSolver *solver, int size);
void generic_add_hidden_layer(NeuralNetworkSolver *solver, int size, int activation_function_flag);
void generic_add_output_layer(NeuralNetworkSolver *solver, int size);

int solver_get_num_layers(NeuralNetworkSolver *solver);
int solver_get_layer_n_val(NeuralNetworkSolver *solver, int layer_number);
int solver_get_layer_m_val(NeuralNetworkSolver *solver, int layer_number);
Matrix *solver_get_output_layer(NeuralNetworkSolver *solver);
#endif


/**
 * 
 * def prime_lists(n -> upper_lim) -> List:
 *      if n < 2: return []
 *         
 *      ret = [2]
 *      for i in range(int(sqrt(n))):
 *          prime = True
 *          for j in ret:
 *              if i % j == 0:
 *                  prime = False
 *                  break
 *          if prime:
 *              ret.append(i)
 *      return ret
 **/
