///////////////////////////////////////////////////////////////////////////                                                                     
// _macros_innfos.h
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

#ifndef _MACROS_INNFOS_H
#define _MACROS_INNFOS_H

///////////////////////////////////////////////////////////////////////
// VELOCITY mode:
///////////////////////////////////////////////////////////////////////

#define INIT_MODE_VEL \
	float  ampl_theta_rad_ref_fl = 0; \ 
	\
	printf("\n"); \
	printf("Enter reference angle amplitude (rad): "); \  
	scanf("%f", &ampl_theta_rad_ref_fl); \  
	AMPL_THETA_RAD_REF = (double)ampl_theta_rad_ref_fl; \
	\
	printf("\n"); \
	printf("AMPL_THETA_RAD_REF = [%lf]\n", AMPL_THETA_RAD_REF); \
	printf("\n");

#define CTRL_MODE_VEL \
	dt_theta_ref = AMPL_THETA_RAD_REF*omega_ref*sin(omega_ref*t_s); \
	\
	if (run_on > 0) \
		set_vel_innfos(dt_theta_ref, &motor, NODE_MOTOR); \
	else \
		set_vel_innfos(0, &motor, NODE_MOTOR); 

///////////////////////////////////////////////////////////////////////
// CURRENT mode:
///////////////////////////////////////////////////////////////////////

#define INIT_MODE_CURR \
	float  ampl_curr_ref_fl = 0; \  
	\
	printf("\n"); \
	printf("Enter reference current amplitude (A): "); \  
	scanf("%f", &ampl_curr_ref_fl); \  
	ampl_curr_ref = (double)ampl_curr_ref_fl; \
	\
	printf("\n"); \
	printf("ampl_curr_ref = [%lf]\n", ampl_curr_ref); \
	printf("\n");

#define CTRL_MODE_CURR \
	curr_ref = ampl_curr_ref*sin(omega_ref*t_s); \
	\
	if (run_on > 0) \
		motor.set_cur_setpoint(NODE_MOTOR, curr_ref); \
	else  \
		motor.set_cur_setpoint(NODE_MOTOR, 0);

///////////////////////////////////////////////////////////////////////
// P_CTRL mode:
///////////////////////////////////////////////////////////////////////

#define INIT_MODE_P_CTRL \
	float  ampl_theta_rad_ref_fl = 0; \ 
	\
	printf("\n"); \
	printf("Enter reference angle amplitude (rad): "); \  
	scanf("%f", &ampl_theta_rad_ref_fl); \  
	AMPL_THETA_RAD_REF = (double)ampl_theta_rad_ref_fl; \
	\
	float  freq_ref_hz_fl = 0; \ 
	\
	printf("\n"); \
	printf("Enter input frequency in Hz (0 for step response): "); \  
	scanf("%f", &freq_ref_hz_fl); \  
	FREQ_REF_HZ = (double)freq_ref_hz_fl; \
	\
	float	K_p_fl = 0; \
	\
	printf("\n"); \
	printf("Enter proportional gain: "); \  
	scanf("%f", &K_p_fl); \ 
	K_p = (double)K_p_fl; \
	\
	printf("\n"); \
	printf("AMPL_THETA_RAD_REF = [%lf]\n", AMPL_THETA_RAD_REF); \
	printf("\n"); \
	printf("K_p = [%lf]\n", K_p); \
	printf("\n");

#define CTRL_MODE_P_CTRL \
	if (run_on > 0) { /* && t_s >= t_step_s) */  \
		if (omega_ref > 0) \
			theta_ref = AMPL_THETA_RAD_REF*sin(omega_ref*t_s); \
		else \
			theta_ref = AMPL_THETA_RAD_REF; \
		curr_ref  = K_p*(theta_ref - theta_m); \
	} \
	else { \
		theta_ref = 0; \
		curr_ref  = 0; \
	} \
	motor.set_cur_setpoint(NODE_MOTOR, curr_ref);   


#endif	

