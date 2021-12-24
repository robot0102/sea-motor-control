///////////////////////////////////////////////////////////////////////////                                                                     
// digital_filters.c
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

#include "digital_filters.h"

void
tf_discr_io_ord1(double* y, double *b, double *a, double x, double *x_prev, double *y_prev) {

	*y =  1.0/a[0]*(b[0]*x + b[1]*(*x_prev) - a[1]*(*y_prev));

	*x_prev =  x;
	*y_prev = *y;

	return NULL;
}

void
TDE_ord1(double *d_est, double input_gain, double dt_y, double input, double *dt_y_prev, double *input_prev){

    *d_est = *dt_y_prev - input_gain*(*input_prev);
	  
	*dt_y_prev  = dt_y;
    *input_prev = input;

	return NULL;
}

void
deflection_Newton(double *deflection, double h_prev, double tau_desire, double pd_h_prev, double deflection_prev){

     *deflection = deflection_prev - (h_prev - tau_desire) / pd_h_prev;
 
	return NULL;
}

void
tf_discr_io_help(double* y, double *b, double *a, double x, double *x_arr, double *y_arr, int ORD) {

	// NOTE: numerator coeffs = b, denominator coeffs = a, filter order = ORD

	// Filtering operation:         
	// Algorithm:
	// a[0]*y[0] = b[0]*x[0] + b[1]*x[1] + ... + b[ORD]*x[ORD]
	//                       - a[1]*y[1] - ... - a[ORD]*y[ORD]

	// Update input vectors:
	int del_i;
	for (del_i = ORD; del_i >= 1; del_i--)
		x_arr[del_i] = x_arr[del_i - 1];
	x_arr[0] = x;

	// Filter output computation:
	double summ = 0;
	for (del_i = 0; del_i <= ORD; del_i++) 
		summ = summ + b[del_i]*x_arr[del_i];  

	for (del_i = 1; del_i <= ORD; del_i++)
		summ = summ - a[del_i]*y_arr[del_i]; 

	*y = 1.0/a[0]*summ; 

	// Update output vectors:
	for (del_i = ORD; del_i >= 2; del_i--)
		y_arr[del_i] = y_arr[del_i - 1]; 

	y_arr[1] = *y; // Note y_arr[0] is always unused  

	/*
	int i = 0;
	printf("b = [");
	for (i = 0; i <= ORD ; i++) 
		printf("%lf  ", b[i]);
	printf("]\n");
	printf("a = [");
	for (i = 0; i <= ORD ; i++) 
		printf("%lf  ", a[i]);
	printf("]\n\n");

	printf("x     = [%lf]\n", x);
	printf("x_arr = [");
	for (i = 0; i <= ORD ; i++) 
		printf("%lf  ", x_arr[i]);
	printf("]\n");
	printf("y_arr = [");
	for (i = 0; i <= ORD ; i++) 
		printf("%lf  ", y_arr[i]);
	printf("]\n");
	printf("y =     [%lf]\n", *y);
	printf("\n");
	*/

	return NULL;
}

void
coeffs_tf_discr_lopass_tustin(double *b, double *a, double K_o, double p_1, double dt) {
	// K_o: TF gain
	// p_1: TF pole 

    b[0] = (K_o*dt)/(dt*p_1 + 2.0); 
    b[1] = (K_o*dt)/(dt*p_1 + 2.0);      // x z^(-1)
 
    a[0] = 1.0;
    a[1] = (dt*p_1 - 2.0)/(dt*p_1 + 2.0);      // x z^(-1)

	return NULL;
}

void
coeffs_tf_discr_pole_zero_tustin(double *b, double *a, double K_o, double p_1, double ze_1, double dt) {

    b[0] = (K_o*(dt*ze_1 + 2.0))/(dt*p_1 + 2.0);
    b[1] = (K_o*(dt*ze_1 - 2.0))/(dt*p_1 + 2.0);      // x z^(-1)
 
    a[0] = 1.0;
    a[1] = (dt*p_1 - 2.0)/(dt*p_1 + 2.0);      // x z^(-1)

	return NULL;
}

void
coeffs_tf_discr_2pole_tustin(double *b, double *a, double K_o, double p_1, double p_2, double dt) {

    b[0] = (K_o*pow(dt,2))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));
    b[1] = (2.0*K_o*pow(dt,2))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-1)
    b[2] = (K_o*pow(dt,2))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-2)
 
    a[0] = 1.0;
    a[1] = (2.0*pow(dt,2)*p_1*p_2 - 8.0)/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-1)
    a[2] = ((dt*p_1 - 2.0)*(dt*p_2 - 2.0))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-2)

	return NULL;
}

void
coeffs_tf_discr_2pole_2zero_tustin(double *b, double *a, double K_o, double ze_1, double ze_2, double p_1, double p_2, double dt) {
	// K_o: TF gain
	// ze_1, ze_2: TF zeros
	// p_1, p_2: TF poles
 
    b[0] = (K_o*(dt*ze_1 + 2.0)*(dt*ze_2 + 2.0))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));
    b[1] = (K_o*(2.0*pow(dt,2)*ze_1*ze_2 - 8.0))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-1)
    b[2] = (K_o*(dt*ze_1 - 2.0)*(dt*ze_2 - 2.0))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-2)
 
    a[0] = 1.0;
    a[1] = (2.0*pow(dt,2)*p_1*p_2 - 8.0)/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-1)
    a[2] = ((dt*p_1 - 2.0)*(dt*p_2 - 2.0))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-2)

	return NULL;
}

void
coeffs_tf_discr_2pole_1zero_tustin(double *b, double *a, double K_o, double ze_1, double p_1, double p_2, double dt) {
	// K_o: TF gain
	// ze_1: TF zeros
	// p_1, p_2: TF poles
 
    b[0] = (K_o*dt*(dt*ze_1 + 2.0))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));
    b[1] = (2.0*K_o*pow(dt,2)*ze_1)/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-1)
    b[2] = (K_o*dt*(dt*ze_1 - 2.0))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-2)
 
    a[0] = 1.0;
    a[1] = (2.0*pow(dt,2)*p_1*p_2 - 8.0)/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-1)
    a[2] = ((dt*p_1 - 2.0)*(dt*p_2 - 2.0))/((dt*p_1 + 2.0)*(dt*p_2 + 2.0));      // x z^(-2)
}

void
coeffs_tf_discr_numord2_denord2_tustin(double *b, double *a, double K_o, double b_1, double b_2, double a_1, double a_2, double dt) {
	// K_o: gain 
	// b_1 b_2: numerator coeffs 
	// a_1 a_2: denominator coeffs 

    b[0] = (K_o*(2.0*b_1*dt + b_2*pow(dt,2) + 4.0))/(2.0*a_1*dt + a_2*pow(dt,2) + 4.0);
    b[1] = (K_o*(2.0*b_2*pow(dt,2) - 8.0))/(2.0*a_1*dt + a_2*pow(dt,2) + 4.0);      // x z^(-1)
    b[2] = (K_o*(b_2*pow(dt,2) - 2.0*b_1*dt + 4.0))/(2.0*a_1*dt + a_2*pow(dt,2) + 4.0);      // x z^(-2)
 
    a[0] = 1.0;
    a[1] = (2.0*a_2*pow(dt,2) - 8.0)/(2.0*a_1*dt + a_2*pow(dt,2) + 4.0);      // x z^(-1)
    a[2] = (a_2*pow(dt,2) - 2.0*a_1*dt + 4.0)/(2.0*a_1*dt + a_2*pow(dt,2) + 4.0);      // x z^(-2)
}

