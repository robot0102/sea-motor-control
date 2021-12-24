///////////////////////////////////////////////////////////////////////////                                                                     
// digital_filters.h
//
// Author: Gabriel Aguirre-Ollinger 
// Documentation start: 06.03.2020
// 
// Description:		 
//				Assorted discrete-time transfer functions	 
//				[coeffs_tf_discr_] functions are generated with MATLAB script test_c2d_coeffs_tustin.m
//					 
// Modifications record:
//		
//
///////////////////////////////////////////////////////////////////////////

#ifndef DIGITAL_FILTERS_H
#define DIGITAL_FILTERS_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

void
tf_discr_io_ord1(double* y, double *b, double *a, double x, double *x_prev, double *y_prev);

void
TDE_ord1(double *d_est, double input_gain, double dt_y, double input, double *dt_y_prev, double *input_prev);

void
deflection_Newton(double *deflection, double h_prev, double tau_feedback, double pd_h_prev, double deflection_prev);

void
tf_discr_io_help(double* y, double *b, double *a, double x, double *x_arr, double *y_arr, int ORD);

void
coeffs_tf_discr_lopass_tustin(double *b, double *a, double K_o, double p_1, double dt);

void
coeffs_tf_discr_pole_zero_tustin(double *b, double *a, double K_o, double p_1, double ze_1, double dt);

void
coeffs_tf_discr_2pole_tustin(double *b, double *a, double K_o, double p_1, double p_2, double dt);

void
coeffs_tf_discr_2pole_2zero_tustin(double *b, double *a, double K_o, double ze_1, double ze_2, double p_1, double p_2, double dt);

void
coeffs_tf_discr_2pole_1zero_tustin(double *b, double *a, double K_o, double ze_1, double p_1, double p_2, double dt);

void
coeffs_tf_discr_numord2_denord2_tustin(double *b, double *a, double K_o, double b_1, double b_2, double a_1, double a_2, double dt);

#endif