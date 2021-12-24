///////////////////////////////////////////////////////////////////////////                                                                     
// control_funcs.c
//
// Author: Gabriel Aguirre-Ollinger 
// Documentation start: 06.03.2020
// 
// Description:		 
//					 
//					 
// Modifications record:
//		
//
///////////////////////////////////////////////////////////////////////////

#include "control_funcs.h"
	
void
coeffs_tf_cont_inv_intadmitt(double* K_inv, double* z_inv_1, double* z_inv_2, double* p_inv_1, double* p_inv_2, 
						double K_pl, double p_pl, double w_real) {

	// Assumptions:
	// 
	// Order-1 plant transfer function is the integral admittance  K_pl/s/(s + p_pl);
	// "Realizability" ord-2 filter with cutoff freq w_real
	//
	// Output:
	// 
	// Coefficients of the DOB transfer function with form K_inv*s*(s + z_inv_2)/(s + p_inv_1)/((s + p_inv_2)

	*K_inv   = w_real*w_real/K_pl;  

	*z_inv_1 = 0; 
	*z_inv_2 = p_pl; 

	*p_inv_1 = w_real; 
	*p_inv_2 = w_real; 
}

// Disturbance observer (SIMPLE INVERSE) input/output function: 
/* 
void 
inv_dob_tf_discr_io(double* dist_in, double pl_in, double pl_out, double* B_inv, double* A_inv, int ORD) {
	// pl_in: plant input
	// pl_out: plant output
	// inv_out: DOB "inverse" block output

	static int init = 1;
	static double *pl_out_arr; 
	static double *inv_out_arr; 
	double inv_out;

	if (init) {
		pl_out_arr  = (double*)malloc((ORD + 1) * sizeof(double)); 
		inv_out_arr = (double*)malloc((ORD + 1) * sizeof(double)); 

		init = 0;
	}

	// DOB block output:
	tf_discr_io_help(&inv_out, B_inv, A_inv, pl_out, pl_out_arr, inv_out_arr, ORD);

	// printf("inv_out = [%3.6lf]\n", inv_out);

	// Disturbance observer output:
	*dist_in = -pl_in + inv_out;
}
*/