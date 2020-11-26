#include "acado_auxiliary_functions.h"
#include "acado_common.h"
#include <stdio.h>
#include <float.h>


/* function prototype */
void di_sim(float* x0_input, float* u,uint32_t delta_t_micro);
void mpc_initialization(void);
real_t acado_solver_step(float* x0_input);