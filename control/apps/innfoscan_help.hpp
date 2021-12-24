///////////////////////////////////////////////////////////////////////////                                                                     
// innfoscan_help.hpp
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

#ifndef INNFOSCAN_HELP_H
#define INNFOSCAN_HELP_H

#include "innfos_can_functions.hpp"

///////////////////////////////////////////////////////////////////////////  
// MOTOR parameters:
///////////////////////////////////////////////////////////////////////////

#define TRQ_COEFF_NM_PER_AMP_SPEC 0.6    // Torque coefficient(Nm/A) QDD-NU80-6 motor property

// Current-to-velocity transfer function parameters: 
// Source data: _data_p_ctrl_step_pos_v1, _data_p_ctrl_step_pos_v2, _data_p_ctrl_step_pos_v3 

// System ID results from MATLAB script test_sys_id_innfos_motor.m: 
/*
#define I_MOMT_IO_SCALED 0.0054984  // "scaled" moment of inertia
#define B_DAMP_IO_SCALED 0.0099418  // "scaled" damping coefficient
#define B_DAMP_IO_SCALED 0.0        // "scaled" Coulomb friction
*/

// MOTOR system ID results from MATLAB script find_params_innfos_motor_v2_fric.m (Coulomb friction considered)
/*
#define I_MOMT_IO_SCALED 0.0055181
#define B_DAMP_IO_SCALED 0.0040887
#define FRIC_IO_SCALED   0.075542

#define I_MOMT_IO		(I_MOMT_IO_SCALED*TRQ_COEFF_NM_PER_AMP_SPEC)  // "true" moment of inertia (kgm^2)
#define B_DAMP_IO		(B_DAMP_IO_SCALED*TRQ_COEFF_NM_PER_AMP_SPEC)  // "true" damping coefficient (N*m/(rad/s))
#define FRIC_IO         (FRIC_IO_SCALED*  TRQ_COEFF_NM_PER_AMP_SPEC)  // "true" Coulomb friction coefficient (N*m)
*/

// SEA system ID results from MATLAB script find_params_imped_SEA_innfos_scaled.m (Coulomb friction considered)
/*
#define I_SEA_IO    0.0053003
#define B_SEA_IO    0.0095384
#define K_SEA_IO    4.3328
#define FRIC_SEA_IO 0.39987
*/

// New spring arrangement (05.03.2020):
/*
#define I_SEA_IO    0.0058882
#define B_SEA_IO    0.0049073
#define K_SEA_IO    4.1395
#define FRIC_SEA_IO 0.41653
*/

// After independent spring constant and K_tau ID using 
// test_sys_id_innfos_motor_v3_ktau_kspring_given.m / find_params_imped_SEA_innfos_scaled_v2_k_spring_est() :

#define I_SEA_IO    0.2063//0.0023437
#define B_SEA_IO    0.4088//0.012257
#define K_SEA_IO    0.4599//0.89804

#define K_TAU_SEA   0.6 //0.45182
#define FRIC_SEA_IO 0.24

///////////////////////////////////////////////////////////////////////////
// Safety limits:
///////////////////////////////////////////////////////////////////////////

#define CURR_AMP_MAX  6.0

///////////////////////////////////////////////////////////////////////////
// MOTOR kinematics:
///////////////////////////////////////////////////////////////////////////

#define PI 3.14159265358979 //HACK: multiple definitions

#define RADSEC_2_RPM     (1.0/2/PI*60)
#define GEAR_RATIO		  6.0                      // QDD-NU80-6 motor property
#define RADSEC_2_VELCMD  (RADSEC_2_RPM*GEAR_RATIO) // convert rad/s to motor velocity command

#define REV_2_RAD			(2*PI)
#define POSREAD_2_RAD		(REV_2_RAD/GEAR_RATIO)

///////////////////////////////////////////////////////////////////////////
// CAN parameters:
///////////////////////////////////////////////////////////////////////////

#define CAN_CHAN "can0"
#define NODE_MOTOR 24 // was 5

///////////////////////////////////////////////////////////////////////////
// Motor functions:
///////////////////////////////////////////////////////////////////////////

double 
read_pos_innfos(controller* motor, int node_motor); 

double 
read_vel_innfos(controller* motor, int node_motor); 

void
set_vel_innfos(double vel_radsec_ref, controller* motor, int node_motor);

void 
torque_fric_coul(double* tau_fric, double fric_coeff, double vel, double N_t, double vel_thr);

#endif