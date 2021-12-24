///////////////////////////////////////////////////////////////////////////                                                                     
// SEA_model.c
//
// Author: Yuepeng Qian
// Documentation start: 23.06.2020
// 
// Description:		 
//					 
//					 
// Modifications record:
//		
//
///////////////////////////////////////////////////////////////////////////

#include "SEA_model.h"

double stiff_spring      =  11.9e3; //N/m
double orginal_length    =  28.5e-3;  // meter
double redius_inner      =  24.5e-3; 
double redius_outer      =  0;
double spring_length1    =  0;
double spring_length2    =  0;
double spring_angle1     =  0;
double spring_angle2     =  0;
double N_pairs_spring    =  6;
double pi                =  3.14159265358979;

double
tau_est_SEA_model(double pre_ex_length, double offset_angle, double diff_angle_sea) {
	
	double tau_sea_est		 =  0;

	redius_outer = redius_inner + orginal_length + pre_ex_length;
	spring_angle1  = diff_angle_sea + offset_angle;
	spring_angle2  = diff_angle_sea - offset_angle;
	spring_length1 = sqrt ( pow(redius_inner, 2) + pow(redius_outer,2) - 2*redius_outer*redius_inner* cos(spring_angle1) );
	spring_length2 = sqrt ( pow(redius_inner, 2) + pow(redius_outer,2) - 2*redius_outer*redius_inner* cos(spring_angle2) );

	tau_sea_est = (double) (N_pairs_spring*stiff_spring*( (spring_length1-orginal_length) * (redius_inner*redius_outer* (double) sin( spring_angle1 )/spring_length1)
		                                                + (spring_length2-orginal_length) * (redius_inner*redius_outer* (double) sin( spring_angle2 )/spring_length2) ) );

	return tau_sea_est;
}

double
diff_h_theta_ord1(double pre_ex_length, double offset_angle,double diff_angle_sea){
	
	double dh_dtheta_ord1 	 =  0;

	redius_outer   = redius_inner + orginal_length + pre_ex_length;
	spring_angle1  = diff_angle_sea + offset_angle;
	spring_angle2  = diff_angle_sea - offset_angle;
	spring_length1 = sqrt ( pow(redius_inner, 2) + pow(redius_outer,2) - 2*redius_outer*redius_inner* cos(spring_angle1) );
	spring_length2 = sqrt ( pow(redius_inner, 2) + pow(redius_outer,2) - 2*redius_outer*redius_inner* cos(spring_angle2) );
	
	dh_dtheta_ord1 =  (double) N_pairs_spring*stiff_spring*redius_inner*redius_outer*( (1- (double) orginal_length/spring_length1) * cos(spring_angle1) + redius_inner*redius_outer*orginal_length*pow(spring_length1,-3)*pow(sin(spring_angle1),2)
	                                                                                 + (1- (double) orginal_length/spring_length2) * cos(spring_angle2) + redius_inner*redius_outer*orginal_length*pow(spring_length2,-3)*pow(sin(spring_angle2),2) );
    return dh_dtheta_ord1;
}

double
diff_h_theta_ord2(double pre_ex_length, double offset_angle,double diff_angle_sea){
	
	double dh_dtheta_ord2    =  0;

	redius_outer = redius_inner + orginal_length + pre_ex_length;
	spring_angle1  = diff_angle_sea + offset_angle;
	spring_angle2  = diff_angle_sea - offset_angle;
	spring_length1 = sqrt ( pow(redius_inner, 2) + pow(redius_outer,2) - 2*redius_outer*redius_inner* cos(spring_angle1) );
	spring_length2 = sqrt ( pow(redius_inner, 2) + pow(redius_outer,2) - 2*redius_outer*redius_inner* cos(spring_angle2) );

	dh_dtheta_ord2 = (double) N_pairs_spring*stiff_spring*redius_inner*redius_outer*( -(1- (double) orginal_length/spring_length1) * sin(spring_angle1) + 3.0*redius_inner*redius_outer*orginal_length*pow(spring_length1,-3)*sin(spring_angle1)*cos(spring_angle1) - 3.0*pow(redius_inner,2)*pow(redius_outer,2)*orginal_length*pow(spring_length1,-5)*pow(sin(spring_angle1),3)
	                                                                                  -(1- (double) orginal_length/spring_length2) * sin(spring_angle2) + 3.0*redius_inner*redius_outer*orginal_length*pow(spring_length2,-3)*sin(spring_angle2)*cos(spring_angle2) - 3.0*pow(redius_inner,2)*pow(redius_outer,2)*orginal_length*pow(spring_length1,-5)*pow(sin(spring_angle2),3) );

}

double
sign(double a){

	double sgn = 0;

	if(abs(a)<0.1)
		sgn=10.0*a;
	else
	{
		if(a>0)
			sgn=1;
		else
			sgn=-1;
	}
	return sgn;
}

