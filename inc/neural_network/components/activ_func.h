#ifndef _ACTIV_FUN_CONST_
#define _ACTIV_FUN_CONST_
#include "libremodel.h"
void activ_fun_set_fun(void (**func_point)(Matrix *, Matrix *), Matrix *(**d_activation_function)(Matrix *, Matrix *), int flag);

#endif
