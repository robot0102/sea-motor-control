///////////////////////////////////////////////////////////////////////////                                                                     
// SEA_model.h
//
// Author: Yuepeng Qian 
// Documentation start: 23.06.2020
// 
// Description:		 
//					 
// Modifications record:
//		
//
///////////////////////////////////////////////////////////////////////////

#ifndef SEA_MODEL_H
#define SEA_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

double
tau_est_SEA_model(double pre_ex_length, double offset_angle, double diff_angle_sea);

double
diff_h_theta_ord1(double pre_ex_length, double offset_angle, double diff_angle_sea);

double
diff_h_theta_ord2(double pre_ex_length, double offset_angle, double diff_angle_sea);

double
sign(double a);

#endif