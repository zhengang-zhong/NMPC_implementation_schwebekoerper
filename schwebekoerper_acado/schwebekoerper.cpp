/*
 *    This file is part of ACADO Toolkit.
 *
 *    ACADO Toolkit -- A Toolkit for Automatic Control and Dynamic Optimization.
 *    Copyright (C) 2008-2014 by Boris Houska, Hans Joachim Ferreau,
 *    Milan Vukov, Rien Quirynen, KU Leuven.
 *    Developed within the Optimization in Engineering Center (OPTEC)
 *    under supervision of Moritz Diehl. All rights reserved.
 *
 *    ACADO Toolkit is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    ACADO Toolkit is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with ACADO Toolkit; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <acado_code_generation.hpp>
#include <math.h>       

USING_NAMESPACE_ACADO


/*Macro  */
#define STATE0_REF 1.0
#define STATE1_REF 0.0
#define STATE2_REF 48.760258862
#define INPUT_REF  157.291157619


int main(int argc, char * const argv[ ])
{
	//
	// Variables
	//

	DifferentialState	x0,x1,x2;
	//AlgebraicState		mu;
	Control				u;



    const double A_B = 2.8274e-3;  // [m**2]
    const double A_SP = 0.4299e-3;  // [m**2]
	const double m = 2.8e-3;  // [kg]
	const double g = 9.81;  // [m/(s**2)]
    const double T_M = 0.57;  // [s]
    const double k_M = 0.31;  // [s**-1]
    const double k_V = 6e-5;  // [m**3]
    const double k_L = 2.18e-4; //# [kg/m]
    const double eta_0 = 1900 / 60 ; 

//	IntermediateState	c, dc;
//
//	const double		m = 1.0;
//	const double		mc = 1.0;
//	const double		L = 1.0;
//	const double		g = 9.81;
//	const double		p = 5.0;
//
//	c	= 0.5 * ((x - w) * (x - w) + y * y - L * L);
//	dc	= dy * y + (dw - dx) * (w - x);

	//
	// Differential algebraic equation
	//

	DifferentialEquation f;

	f << dot( x0 ) == x1;
	f << dot( x1 ) ==  k_L / m * ((k_V * (x2 + eta_0) - A_B * x1) / A_SP)* ((k_V * (x2 + eta_0) - A_B * x1) / A_SP) - g;
	f << dot( x2 ) ==  -1 / T_M * x2 + k_M / T_M * u;
	//
	// Weighting matrices and reference functions
	//

	Function rf;
	Function rfN;

	rf << x0 << x1 << x2 << u;
	rfN << x0 << x1 << x2;

	//DMatrix W = eye<double>( rf.getDim() ) * 5;
	//weight matrix for opt problem
	DMatrix W=eye<double>(rf.getDim());
	W(0,0) = 1e5;
	W(1,1) = 1;
	W(2,2) = 1;
	W(3,3) = 1;

	DMatrix WN = eye<double>( rfN.getDim() ) * 10;
	WN(0,0) = 1e5;
	WN(1,1) = 1;
	WN(2,2) = 1;

	//W(0,0) = 1e5;
	//W(1,1) = 1;
	//W(2,2) = 1;
	
	
	//reference
/* 	DVector v(4);
	v.setAll(0);
	v(0) = 1.8;
	v(1) = 48.760258862;
	v(3) = 157.291157619; */

	const int N  = 10;
	const int Ni = 4;
	const double Ts = 0.1;

	OCP ocp(0, N * Ts, N);

	ocp.subjectTo( f );

	ocp.minimizeLSQ(W, rf);
	ocp.minimizeLSQEndTerm(WN, rfN);
	
	
	//ocp.subjectTo( AT_END , x0 == STATE0_REF); 
	//ocp.subjectTo( AT_END , x1 == STATE1_REF); 
	//ocp.subjectTo( AT_END , x2 == STATE2_REF); 
	

	ocp.subjectTo(-255 <= u <= 255);
	ocp.subjectTo( 0 <= x0 <= 1.8 );
	ocp.subjectTo( -1 <= x1 <= 1  );
	ocp.subjectTo( 0 <= x2 <= 200 );

	//
	// Export the code:
	//
	OCPexport mpc( ocp );

	mpc.set(HESSIAN_APPROXIMATION, GAUSS_NEWTON);
	//mpc.set(DISCRETIZATION_TYPE, MULTIPLE_SHOOTING);
	mpc.set(DISCRETIZATION_TYPE, SINGLE_SHOOTING);

	mpc.set(INTEGRATOR_TYPE, INT_RK3);
	mpc.set(NUM_INTEGRATOR_STEPS, N * Ni); //along prediction horizon

	mpc.set(SPARSE_QP_SOLUTION, FULL_CONDENSING);
	//mpc.set(SPARSE_QP_SOLUTION, CONDENSING);
	mpc.set(FIX_INITIAL_STATE, YES);

//	mpc.set(SPARSE_QP_SOLUTION, CONDENSING);
	mpc.set(QP_SOLVER, QP_QPOASES);
//	mpc.set(MAX_NUM_QP_ITERATIONS, 20);
	mpc.set(HOTSTART_QP, YES);

//	mpc.set(SPARSE_QP_SOLUTION, SPARSE_SOLVER);
//	mpc.set(QP_SOLVER, QP_FORCES);

//	mpc.set(LEVENBERG_MARQUARDT, 1.0e-10);

	mpc.set(GENERATE_TEST_FILE, YES);
	mpc.set(GENERATE_MAKE_FILE, YES);
	mpc.set(GENERATE_MATLAB_INTERFACE, NO);

//	mpc.set(USE_SINGLE_PRECISION, YES);
//	mpc.set(CG_USE_OPENMP, YES);

	if (mpc.exportCode( "schwebekoerper_export" ) != SUCCESSFUL_RETURN)
		exit( EXIT_FAILURE );

	mpc.printDimensionsQP( );

	return EXIT_SUCCESS;
}
