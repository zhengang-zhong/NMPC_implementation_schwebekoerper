
#include "acado_function.h"

/*Macro  */
#define STATE0_REF 1.0
#define STATE1_REF 0.0
#define STATE2_REF 48.760258862
#define INPUT_REF  157.291157619

/* Some convenient definitions. */
#define NX          ACADO_NX  /* Number of differential state variables.  */
#define NXA         ACADO_NXA /* Number of algebraic variables. */
#define NU          ACADO_NU  /* Number of control inputs. */
#define NOD         ACADO_NOD  /* Number of online data values. */

#define NY          ACADO_NY  /* Number of measurements/references on nodes 0..N - 1. */
#define NYN         ACADO_NYN /* Number of measurements/references on node N. */

#define N           ACADO_N   /* Number of intervals in the horizon. */

#define NUM_STEPS   10        /* Number of real-time iterations. */
#define VERBOSE     0         /* Show iterations: 1, silent: 0.  */

#define SECOND_TO_MICROSECOND 1e6
#define MILISECOND_TO_MICROSSECOND 1e3
/* Global variables used by the solver. */
ACADOvariables acadoVariables;
ACADOworkspace acadoWorkspace;

const float A_B = 2.8274e-3;  // [m**2]
const float A_SP = 0.4299e-3;  // [m**2]
const float m = 2.8e-3;  // [kg]
const float g = 9.81;  // [m/(s**2)]
const float T_M = 0.57;  // [s]
const float k_M = 0.31;  // [s**-1]
const float k_V = 6e-5;  // [m**3]
const float k_L = 2.18e-4; //# [kg/m]
const float eta_0 = 1900 / 60 ; 



void di_sim(float* x,float* u, uint32_t delta_t_micro){
	float temp_x0=0.0,temp_x1=0.0,temp_x2=0.0;
	float delta_t =(float)(delta_t_micro)/SECOND_TO_MICROSECOND;
	printf("float deltat is %f, x[2] is %f\n\r",delta_t,x[2]);
	temp_x0 = x[0] + x[1]*delta_t;
	temp_x1 = x[1] + (k_L / m * ((k_V * (x[2] + eta_0) - A_B * x[1]) / A_SP)* ((k_V * (x[2] + eta_0) - A_B * x[1]) / A_SP) - g)*delta_t;
	temp_x2 = x[2] + (-1 / T_M * x[2] + k_M / T_M * u[0])*delta_t;
	printf("x[0] is %f, x[1] is %f,x[2] is %f\n\r",temp_x0,temp_x1,temp_x2);
	x[0] = temp_x0 ;
	x[1] = temp_x1 ;
	x[2] = temp_x2 ;
}


void mpc_initialization(void){
	/* Some temporary variables. */
	int    i, iter;
	float  u_return;
	acado_timer t;

	/* Initialize the solver. */
	acado_initializeSolver();

	/* Initialize the states and controls. */
	for (i = 0; i < NX * (N + 1); ++i)  acadoVariables.x[ i ] = 0.0;
	for (i = 0; i < NU * N; ++i)  acadoVariables.u[ i ] = 0.0;

	/* Initialize the measurements/reference. */
	for (i = 0; i < N; ++i)  {
		acadoVariables.y[ i*NY ] = STATE0_REF;
		acadoVariables.y[ i*NY+1 ] = STATE1_REF;
		acadoVariables.y[ i*NY+2] =  STATE2_REF;
		acadoVariables.y[ i*NY+3] =  INPUT_REF;
	}
	acadoVariables.yN[ 0 ] = STATE0_REF;
	acadoVariables.yN[ 1 ] = STATE1_REF;
	acadoVariables.yN[ 2 ] =  STATE2_REF;

	/* MPC: initialize the current state feedback. */
#if ACADO_INITIAL_STATE_FIXED
	acadoVariables.x0[ 0 ] = 0.0;
	acadoVariables.x0[ 1 ] = 0.0;
	acadoVariables.x0[ 2 ] = 50.0;
#endif	

}

real_t acado_solver_step(float* x0_input){
	/* Some temporary variables. */
	int    i, iter;
	float  u_return;
	acado_timer t;
	/* read current state. */
	acadoVariables.x0[ 0 ] = x0_input[0];
	acadoVariables.x0[ 1 ] = x0_input[1];
	acadoVariables.x0[ 2 ] = x0_input[2];

	/* Prepare first step */
	acado_preparationStep();

	/* Get the time before start of the loop. */
	// acado_tic( &t );
	/* The "real-time iterations" loop. */
	// for(iter = 0; iter < NUM_STEPS; ++iter)
	// {
    /* Perform the feedback step. */
	acado_feedbackStep();
	// printf("return u1 is : %f\n\r",acadoVariables.u[0]);
	// printf("return u2 is : %f\n\r",acadoVariables.u[1]);

	// printf("return u is : %lf\n\r",acadoVariables.u[0]);
    /* return the first element from the optimization solution. */
	return acadoVariables.u[0];
}
