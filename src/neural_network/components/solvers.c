#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "libremodel.h"
#include "neural_network/neural.h"
#include "neural_network/components/solver.h"
#include "neural_network/components/activ_func.h"


struct _neural_network_hidden_solver{
    NeuralNetwork *network;
    List *layers;
    List *(*init_layers)();
    void (*create_layer)(NeuralNetworkSolver *, int, int);
    void (*step_forward)(void *, Matrix **);
    void (*dz_solver)(void *);
    void (*da_solver)(void *, void *);
    void (*dw_db_solver)(void *, void *, double);
    void (*d_cost)(void *, Matrix *);
    int input_size;
    double alpha, rate;
};

void neural_network_set_solver(NeuralNetworkSolver *solver, NeuralNetwork *network){
    if(solver_check_valid(solver) && network != NULL && solver->hidden_solver->network == NULL && network->solver == NULL){
        network->solver = solver;
        solver->hidden_solver->network = network;
    }
}

char hidden_solver_check_valid(NeuralNetworkHiddenSolver *solver){
    return solver != NULL 
        && solver->network != NULL; //Should have the same network. Would be bad otherwise.
}

int solver_get_num_layers(NeuralNetworkSolver *solver){
    if(!solver_check_valid(solver) || !hidden_solver_check_valid(solver->hidden_solver)) return -1;
    
    return 1 + listGetSize(solver->hidden_solver->layers);
}

int solver_get_layer_n_val(NeuralNetworkSolver *solver, int layer_number){
    if(!solver_check_valid(solver) || !hidden_solver_check_valid(solver->hidden_solver)) return -1;
    
    if(layer_number == 0) return solver->hidden_solver->input_size;
    
    return matrixGetN(*(LIST_DER(Matrix **, listGet(solver->hidden_solver->layers, layer_number - 1))));
}

int solver_get_layer_m_val(NeuralNetworkSolver *solver, int layer_number){
    if(!solver_check_valid(solver) || !hidden_solver_check_valid(solver->hidden_solver)) return -1;
    
    if(layer_number == 0) return solver->hidden_solver->input_size;
    
    return matrixGetM(*(LIST_DER(Matrix **, listGet(solver->hidden_solver->layers, layer_number - 1))));
}

Matrix *solver_get_output_layer(NeuralNetworkSolver *solver){
    if(!solver_check_valid(solver) || !hidden_solver_check_valid(solver->hidden_solver) || !listGetSize(solver->hidden_solver->layers)) return NULL;
    
    return *(LIST_DER(Matrix **, listGet(solver->hidden_solver->layers, listGetSize(solver->hidden_solver->layers) - 1)));
}

void neural_network_forward_propagate(NeuralNetworkSolver *solver, Matrix *input){
    if(input == NULL || !solver_check_valid(solver)) return;
    
    NeuralNetworkHiddenSolver *h_solver = solver->hidden_solver;
    NeuralNetwork *network = h_solver->network;
    List *layers = h_solver->layers;
    unsigned int i = 0;
    unsigned const int lim = listGetSize(layers);
    
    Matrix **a = &input;
    void *flayer;
    //void *flayer = *((void **) listGet(layers, i));
    
    //h_solver->step_forward(network, flayer, a);    
    for(i = 0; i < lim; i++){
        flayer = *((void **) listGet(layers, i));
        h_solver->step_forward(flayer, a);
    }
}

void neural_network_back_propagate(NeuralNetworkSolver *solver, Matrix *input, Matrix *y){
    if(!solver_check_valid(solver) || !hidden_solver_check_valid(solver->hidden_solver) /*Add matrix check later*/) return;
    
    NeuralNetwork *network = solver->hidden_solver->network;
    NeuralNetworkHiddenSolver *h_solver = solver->hidden_solver;
    List *layers = solver->hidden_solver->layers;
    int i = listGetSize(layers) - 1;
    
    void *flayer;
    void *blayer = *((void **) listGet(layers, i));
    
    //Update {blayer->da, blayer:[a,da], y} 
    h_solver->d_cost(blayer, y);
    
    for(i = i - 1; i >= 0; i--){
        flayer = blayer;
        blayer = *((void **) listGet(layers, i));
        
        //Update {flayer->dz, flayer:[z, a, da, dz]}
        h_solver->dz_solver(flayer);
        
        //Update {blayer->da, blayer:[da], flayer:[w,dz]}
        h_solver->da_solver(flayer, blayer);
        
        //Update {[flayer->b, flayer->w], blayer:[a], flayer:[dz,b,w]}
        h_solver->dw_db_solver(flayer, blayer, h_solver->rate);
    }
    flayer = blayer;
    
    //Update {flayer->dz, flayer:[z, a, da, dz]}
    h_solver->dz_solver(flayer);
    
    //Update {[flayer->b, flayer->w], blayer:[a], flayer:[dz,b,w]}
    h_solver->dw_db_solver(flayer, &input, h_solver->rate);
}


/**
 * Generic Functions
 **/

typedef struct generic_neural_layer{
    Matrix *a, *b, *w, *z, *da, *db, *dw, *dz;
     void (* activation_function)(Matrix *, Matrix *);
     Matrix *(* d_activation_function)(Matrix *, Matrix *);
} Generic_Neural_Layer;

void generic_neural_layer_destroyer(void *target){
    Generic_Neural_Layer layer = *((Generic_Neural_Layer *)target);
    
    //Could treat it as a Matrix ** and iterate through the "array" but eh... hardcode!
    
    if(layer.a != NULL) matrixDestroy(layer.a, 0);
    if(layer.b != NULL) matrixDestroy(layer.b, 0);
    if(layer.w != NULL) matrixDestroy(layer.w, 0);
    if(layer.z != NULL) matrixDestroy(layer.z, 0);
    if(layer.da != NULL) matrixDestroy(layer.da, 0);
    if(layer.db != NULL) matrixDestroy(layer.db, 0);
    if(layer.dw != NULL) matrixDestroy(layer.dw, 0);
    if(layer.dz != NULL) matrixDestroy(layer.dz, 0);
}

List *generic_init_layers(){
    return listCreate(2, sizeof(Generic_Neural_Layer *), NULL, generic_neural_layer_destroyer); 
}

void generic_create_layer(NeuralNetworkSolver *solver, int size, int activation_function_flag){
//     (void) solver;
//     (void) size;
    
    /**
     * TODO
     **/
    
    Generic_Neural_Layer *layer = (Generic_Neural_Layer *) calloc(1, sizeof(Generic_Neural_Layer));
    assert(layer != NULL);
    
    layer->a = matrixCreate(size, 1, NULL, size);
    assert(layer->a != NULL);
    layer->da = matrixCreate(size, 1, NULL, size);
    assert(layer->da != NULL);
    layer->b = matrixCreate(size, 1, NULL, size);
    assert(layer->b != NULL);
    layer->db = matrixCreate(size, 1, NULL, size);
    assert(layer->db != NULL);
    layer->z = matrixCreate(size, 1, NULL, size);
    assert(layer->z != NULL);
    layer->dz = matrixCreate(size, 1, NULL, size);
    assert(layer->dz != NULL);
    
    int prev = solver_get_num_layers(solver) - 1;
    
    if(prev){
        prev = solver_get_layer_n_val(solver, prev);
    }else{
        prev = solver->hidden_solver->input_size;
    } 
    
    double values[size * prev];
    
    for(int i = 0; i < size * prev; i++){
        values[i] = ((rand() % 501) - 250) / 100.0;
    }
    
    layer->w = matrixCreate(size, prev, values, size * prev);
    assert(layer->w != NULL);
    
    layer->dw = matrixCreate(size, prev, NULL, size * prev);
    assert(layer->dw != NULL);
    
    activ_fun_set_fun(&(layer->activation_function), &(layer->d_activation_function), activation_function_flag);
    
    listAppend(solver->hidden_solver->layers, &layer);
}

NeuralNetworkSolver *generic_neural_network_solver_create(double alpha, double rate){
    if(alpha < 0 || alpha > 1.0 || rate < 0) return NULL;
    
    NeuralNetworkSolver *ret = (NeuralNetworkSolver *) calloc(1, sizeof(NeuralNetworkSolver));
    assert(ret != NULL);
    ret->hidden_solver = (NeuralNetworkHiddenSolver *) calloc(1, sizeof(struct _neural_network_hidden_solver));
    assert(ret->hidden_solver != NULL);
    
    ret->hidden_solver->alpha = alpha;
    ret->hidden_solver->rate = rate;
    ret->hidden_solver->init_layers = generic_init_layers;
    ret->hidden_solver->create_layer = generic_create_layer;
    
    return ret;
}

void generic_add_input_layer(NeuralNetworkSolver *solver, int size){
    if(solver_check_valid(solver) && hidden_solver_check_valid(solver->hidden_solver)){
        solver->hidden_solver->input_size = size;
        solver->hidden_solver->layers = solver->hidden_solver->init_layers();
        assert(solver->hidden_solver->layers != NULL);
    }
}
void generic_add_hidden_layer(NeuralNetworkSolver *solver, int size, int activation_function_flag){
    if(solver_check_valid(solver) && hidden_solver_check_valid(solver->hidden_solver)){
        solver->hidden_solver->create_layer(solver, size, activation_function_flag);
    }
}
void generic_add_output_layer(NeuralNetworkSolver *solver, int size){
    solver->add_hidden_layer(solver, size, ACTIV_FUNC_SIGMOID);
}


/**
 * Stochastic Gradient Descent
 **/

typedef struct sgd_neural_layer{
    Generic_Neural_Layer super;
} SGD_Neural_Layer;


//Quite honestly considering removing these two functions below....
List *sgd_init_layers(){
    return generic_init_layers();
}

void sgd_create_layer(NeuralNetworkSolver *solver, int size, int activation_function_flag){
    generic_create_layer(solver, size, activation_function_flag);
}

void sgd_step_forward(void *fl, Matrix **a){
    SGD_Neural_Layer *flayer = (SGD_Neural_Layer *) fl;
    
    matrixMul(flayer->super.w, *a, flayer->super.z, 0);
    *a = flayer->super.a;
    matrixAdd(flayer->super.b, flayer->super.z, flayer->super.z, 0);
    flayer->super.activation_function(flayer->super.z, *a);
}

void sgd_dz_solver(void *fl){
    SGD_Neural_Layer *flayer = (SGD_Neural_Layer *) fl;
    
    Matrix * dadz = flayer->super.d_activation_function(flayer->super.z, flayer->super.a);
    matrixMul(dadz, flayer->super.da, flayer->super.dz, 0);
    matrixDestroy(dadz, 0);
}

void sgd_da_solver(void *fl, void *bl){
    matrixMul(((SGD_Neural_Layer *) fl)->super.w, ((SGD_Neural_Layer *) fl)->super.dz, ((SGD_Neural_Layer *) bl)->super.da, MATRIX_A_TRANS);
}

void sgd_dw_dz_solver(void *fl, void *bl, double rate){
    SGD_Neural_Layer *flayer = (SGD_Neural_Layer *) fl;
    
    matrixConstantMul(flayer->super.dz, rate);
    
    matrixMul(flayer->super.dz, ((SGD_Neural_Layer *) bl)->super.a, flayer->super.w, MATRIX_B_TRANS | MATRIX_RESULT_ADD);
    matrixAdd(flayer->super.b, flayer->super.dz, flayer->super.b, 0);
}

void sgd_d_cost(void *bl, Matrix *y){
    matrixSub(y, ((SGD_Neural_Layer *) bl)->super.a, ((SGD_Neural_Layer *) bl)->super.da, 0);
}

NeuralNetworkSolver *neural_network_solver_sgd(double alpha, double rate){
    NeuralNetworkSolver *ret = generic_neural_network_solver_create(alpha, rate);
    //I don't really need dW, dZ, or dB directly. Maybe I should delete them?
    
    //Set the solver functions here to keep the functions unique at each instance
    
    //Builder Functions
    ret->add_input_layer = generic_add_input_layer;
    ret->add_hidden_layer = generic_add_hidden_layer;
    ret->add_output_layer = generic_add_output_layer;
    ret->hidden_solver->init_layers = sgd_init_layers;
    ret->hidden_solver->create_layer = sgd_create_layer;
    
    //Back propagation functions
    ret->backPropagate = neural_network_back_propagate;
    ret->hidden_solver->dz_solver = sgd_dz_solver;
    ret->hidden_solver->da_solver = sgd_da_solver;
    ret->hidden_solver->dw_db_solver = sgd_dw_dz_solver;
    ret->hidden_solver->d_cost = sgd_d_cost;
    
    //Forward propagation functions
    ret->forwardPropagate = neural_network_forward_propagate;
    ret->hidden_solver->step_forward = sgd_step_forward;
    
    //Add unique struct definition here to ret->hidden_solver->solver_data
    
    return ret;
}

    /*
void sgd_forwardPropagate(NeuralNetworkSolver *solver, Matrix *input){
    if(input == NULL || !solver_check_valid(solver)) return;
    
    NeuralNetwork *network = solver->hidden_solver->network;
    List *layers = solver->hidden_solver->layers;
    unsigned int i = 0;
    unsigned const int lim = listGetSize(layers);
    
    Matrix *a, *b, *w, *z;
    Generic_Neural_Layer layer = *(LIST_DER(Generic_Neural_Layer *, listGet(layers, i)));
    
    a = input;
    b = layer.b;
    w = layer.w;
    z = layer.z;
    
    for(i = 1; i < lim; i++){
        matrixMul(w, a, z, 0);
        a = layer.a;
        matrixAdd(b, z, z, 0);
        network->activation_function(z, a);
        
        layer = *(LIST_DER(Generic_Neural_Layer *, listGet(layers, i)));
        b = layer.b;
        w = layer.w;
        z = layer.z;
    }
    
    matrixMul(w, a, z, 0);
    a = layer.a;
    matrixAdd(b, z, z, 0);
    network->activation_function(z, a);
    
    Matrix **a = (Matrix **) listGet(network->a, 0);
    Matrix **a_stop = a + listGetSize(network->a) - 1;  
    Matrix **b = (Matrix **) listGet(network->b, 0); 
    Matrix **w = (Matrix **) listGet(network->w, 0); 
    Matrix **z = (Matrix **) listGet(network->z, 0);
    
    matrixSetMat(*a, 0);
    matrixAdd(*a, input, *a, 0);
    
//     matrixMul(*w, *a, *z, 0);
//     a++;
//     matrixAdd(*b, *z, *z, 0);
//     network->activation_function(*z, *a);
//     b++;
//     w++;
//     z++;
    while(a < a_stop){
        matrixMul(*w, *a, *z, 0);
        a++;
        matrixAdd(*b, *z, *z, 0);
        network->activation_function(*z, *a);
//          *
//          * 
//          * a(n) = act(z(n))
//          * z(n + 1)
//          * 
//          *
        b++;
        w++;
        z++;
    }
    
}*/


    /*
void sgd_backPropagate(NeuralNetworkSolver *solver, Matrix *input, Matrix *y){
    if(!solver_check_valid(solver) || !hidden_solver_check_valid(solver->hidden_solver) *Add matrix check later*) return;
    
    NeuralNetwork *network = solver->hidden_solver->network;
    NeuralNetworkHiddenSolver *h_solver = solver->hidden_solver;
    List *layers = solver->hidden_solver->layers;
    int i = listGetSize(layers) - 1;
    
    Matrix *a, *b, *w, *z, *da, *dz, *dadz;
    SGD_Neural_Layer layer = *(LIST_DER(SGD_Neural_Layer *, listGet(layers, i)));
    a = layer.super.a;
    b = layer.super.b;
    w = layer.super.w;
    z = layer.super.z;
    da = layer.super.da;
    dz = layer.super.dz;
    
    matrixSub(y, a, da, 0);
    
    for(i = i - 1; i >= 0; i--){
        layer = *(LIST_DER(SGD_Neural_Layer *, listGet(layers, i)));
        
        dadz = network->d_activation_function(z, a);
        a = layer.super.a;
        matrixMul(dadz, da, dz, 0);
        matrixDestroy(dadz, 0);
        da = layer.super.da;
        
        matrixMul(w, dz, da, MATRIX_A_TRANS);
        
        matrixConstantMul(dz, h_solver->rate);
        
        matrixMul(dz, a, w, MATRIX_B_TRANS | MATRIX_RESULT_ADD);
        matrixAdd(b, dz, b, 0);
        
        b = layer.super.b;
        w = layer.super.w;
        z = layer.super.z;
        dz = layer.super.dz;        
    }
    
    dadz = network->d_activation_function(z, a);
    matrixMul(dadz, da, dz, 0);
    matrixDestroy(dadz, 0); 
    
    matrixConstantMul(dz, h_solver->rate);
    
    matrixMul(dz, input, w, MATRIX_B_TRANS | MATRIX_RESULT_ADD);
    matrixAdd(b, dz, b, 0);
    
    
    Matrix **a = (Matrix **) listGet(network->a, listGetSize(network->a) - 1);
    Matrix **a_stop = (Matrix **) listGet(network->a, 0);  
    Matrix **b = (Matrix **) listGet(network->b, listGetSize(network->b) - 1); 
    Matrix **w = (Matrix **) listGet(network->w, listGetSize(network->w) - 1); 
    Matrix **z = (Matrix **) listGet(network->z, listGetSize(network->z) - 1);
    Matrix **da = (Matrix **) listGet(h_solver->da, listGetSize(h_solver->da) - 1); 
    Matrix **dz = (Matrix **) listGet(h_solver->dz, listGetSize(h_solver->dz) - 1); 
    
    //dC/da
    matrixSub(y, *a, *da, 0);
//     printf("**************************************************************************************************************************************\n#EXPECTED\n");
//     matrixPrint(y);
//     printf("#GOT\n");
//     matrixPrint(*a);
//     printf("#ERROR\n");
//     matrixPrint(*da);
    a--;
    
    while(a >= a_stop){
//         printf("\n>>>Entering Layer\n#DA:\n");
//         matrixPrint(*da);
        //n*1 = (n * n) * (n * 1)
        // dz = (der_of_act) * (da)
        //dC/dz = dC/da * derivative(activation_function(z))
        Matrix *dadz = network->d_activation_function(*z);
        matrixMul(dadz, *da, *dz, 0);
//         printf("#DADZ:\n");
//         matrixPrint(dadz);
        matrixDestroy(dadz, 0);
        //matrixConstantMul(*da, network->d_activation_function(*z));
        da--;
//         printf("#DZ:\n");
//         matrixPrint(*dz);
        
        //dC/da = w * dC/dz
        matrixMul(*w, *dz, *da, MATRIX_A_TRANS);
        
        //Multiply by learning rate before updating dw and db
        matrixConstantMul(*dz, h_solver->rate);
        
        //w -= dw
        matrixMul(*dz, *a, *w, MATRIX_B_TRANS | MATRIX_RESULT_SUB);
//         printf("#W:\n");
//         matrixPrint(*w);
        
        //b -= db (db == dz * rate)
//         matrixSub(*b, *dz, *b, 0);
//         printf(">>>Exiting Layer\n");
        b--;
        w--;
        a--;
        z--;
        dz--;
    }
    
//     matrixConstantMul(*da, network->d_activation_function(*z));
//     matrixConstantMul(*da, h_solver->rate);
//     matrixSub(*b, *da, *b, 0);
//     matrixMul(*da, input, *w, MATRIX_B_TRANS | MATRIX_RESULT_SUB);
}*/

/**
* Add a struct here for any additional data unique to the solver that isn't contained in the generic struct
**/


