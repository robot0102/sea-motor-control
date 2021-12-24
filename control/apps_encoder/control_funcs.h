///////////////////////////////////////////////////////////////////////////                                                                     
// control_funcs.h
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

#ifndef CONTROL_FUNCS_H
#define CONTROL_FUNCS_H

#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h> 
#include <math.h> 

void
coeffs_tf_cont_inv_intadmitt(double* K_inv, double* z_inv_1, double* z_inv_2, double* p_inv_1, double* p_inv_2, 
							 double K_pl, double p_pl, double w_real);

/*
void
inv_dob_tf_discr_io(double* dist_in, double pl_in, double pl_out, double* B_inv, double* A_inv, int ORD);
*/

#endif