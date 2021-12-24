///////////////////////////////////////////////////////////////////////////                                                                     
// innfoscan_help.cpp
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

#include "innfoscan_help.hpp"

double 
read_pos_innfos(controller* motor, int node_motor) {
	return POSREAD_2_RAD*motor->read_pos_setpoint(node_motor);
}


double 
read_vel_innfos(controller* motor, int node_motor) {
	return 1.0/RADSEC_2_VELCMD*motor->read_vel_setpoint(node_motor); 
}

void
set_vel_innfos(double vel_radsec_ref, controller* motor, int node_motor) {
	motor->set_vel_setpoint(node_motor, RADSEC_2_VELCMD*vel_radsec_ref); 
}

void 
torque_fric_coul(double* tau_fric, double fric_coeff, double vel, double N_t, double vel_thr) {
	// N_t: number of time constants for vel. threshold
	double sign;

	if (vel >= 0)
		sign = -1;
	else
		sign = 1;

	*tau_fric = fric_coeff*sign; 
	// 	*tau_fric = fric_coeff*sign*(1.0 - exp(-N_t*abs(vel)/ vel_thr));
}