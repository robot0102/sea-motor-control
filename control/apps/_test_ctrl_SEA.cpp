///////////////////////////////////////////////////////////////////////////                                                                     
// _test_ctrl_SEA.cpp
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

#include "_test_scripts.hpp" 
#include "_macros_innfos.h"

// SEA kinematics:
#define PULLEY_SEA_RATIO 2.0

extern int run_on; 

void*
test_ctrl_SEA(void* dt_ns_ref) {

	struct period_info pinfo;
	long* dt_ns = (long*)dt_ns_ref;

	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////
	// FILE HANDLING FUNCTIONS:
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////
	// Create data files ID string:
	////////////////////////////////////////////////////////////////////////

	char		data_fname_basic[LEN_NAME_MAX];
	char		time_str[LEN_NAME_MAX];	 

	// Time stamp variables:
	time_t       curr_time;

	curr_time = time(NULL);
	strftime(time_str, LEN_NAME_MAX, "%y%m%d_%H%M%S", localtime(&curr_time));	

	// File name:
	strcpy(data_fname_basic, DATA_FNAME_HEAD);
	strcat(data_fname_basic, time_str);

	///////////////////////////////////////////////////////////////////////////
	// Create data file folder:
	///////////////////////////////////////////////////////////////////////////

	char data_file_dir[LEN_NAME_MAX];

	sprintf(data_file_dir, "%s%s/", DATA_DIR, data_fname_basic);

	if (mkdir(data_file_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
		printf("\nCouldn't create directory (%s)\n\n", data_file_dir); 
		exit(0);
	} 

	//////////////////////////////////////////////////////////////////////// 
	// Create copy of key master file:
	////////////////////////////////////////////////////////////////////////

	static char	key_file_path_master[2*LEN_NAME_MAX];
	static char	key_file_path_copy[2*LEN_NAME_MAX];

	FILE*		key_file_id_master;
	FILE*		key_file_id_copy;

	// Open existing master key file:
	file_path_create(key_file_path_master, BASE_DIR, KEY_FNAME, EXT_TXT);
	key_file_id_master = fopen(key_file_path_master, "r");

	// Create key file copy:
	file_path_create(key_file_path_copy, data_file_dir, data_fname_basic, EXT_KEY);
	key_file_id_copy = fopen(key_file_path_copy, "wb");

	if (key_file_id_master == NULL) {
		printf("Failed to open [%s]", key_file_path_master);
		exit(0);
	}
	if (key_file_id_copy == NULL) {
		printf("Failed to open [%s]", key_file_path_copy);
		exit(0);
	}

	// printf("\nkey_file_path_master = [%s]\n\n", key_file_path_master);
	// printf("\nkey_file_path_copy   = [%s]\n\n", key_file_path_copy);

	// Copy text:
	copy_text_file(key_file_id_master, key_file_id_copy);

	fclose(key_file_id_master);   
	fclose(key_file_id_copy);   

	///////////////////////////////////////////////////////////////////////////
	// Create PARAMETER file:
	///////////////////////////////////////////////////////////////////////////	
	
	char param_file_path[2*LEN_NAME_MAX];
	  
	file_path_create(param_file_path, data_file_dir, data_fname_basic, EXT_PAR); 
	// printf("\nparam_file_path = [%s]\n\n", param_file_path);
	
	FILE* param_file_id = fopen(param_file_path, "wb");

	///////////////////////////////////////////////////////////////////////////
	// Create asynchronous DATA logger:
	///////////////////////////////////////////////////////////////////////////

	char data_file_path[2*LEN_NAME_MAX];
	  
	file_path_create(data_file_path, data_file_dir, data_fname_basic, EXT_DAT); 
	// printf("\ndata_file_path = [%s]\n\n", data_file_path);

	std::shared_ptr<spdlog::logger>  async_log_dat = async_log_init(data_file_path);

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	// HARDWARE INITIALIZATION:
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Initialize Sensoray 526:
	///////////////////////////////////////////////////////////////////////////

	// ADC channels:
    const int NUM_ADC_CHAN			= 6;
    int32_t ADC_CHAN[NUM_ADC_CHAN]	= {2, 3, 4, 5, 6, 7};

    double adc_data[NUM_ADC_CHAN]	= {0, 0, 0, 0, 0, 0};

	// Initialize hardware:
	s526_init();
    s526_adc_init(ADC_CHAN, NUM_ADC_CHAN);

	///////////////////////////////////////////////////////////////////////////
	// Initialize ATI FT_data sensor:
	///////////////////////////////////////////////////////////////////////////

	const int N_FT_OUT		= 6;
	const int FT_INPUT_ON	= 1;

	float adc_data_fl[N_FT_OUT + 1] = {0, 0, 0, 0, 0, 0, 0};
	float FT_data[N_FT_OUT]         = {0, 0, 0, 0, 0, 0};

	init_ft_sensor_ati(CAL_FILE, ADC_CHAN, NUM_ADC_CHAN);

	///////////////////////////////////////////////////////////////////////////
	// Initialize motor:
	///////////////////////////////////////////////////////////////////////////  

    controller motor(CAN_CHAN);
	const double DELAY_SEC = 1.0;

	printf("\n");
	printf("[%s] Enabling motor... \n\n", __FILE__);

    motor.enable_motor(NODE_MOTOR);
	sleep(DELAY_SEC); 
	
	printf("[%s] changing motor mode... \n\n", __FILE__);

	// Initialize INNFOS mode (CRITICAL):
	// motor.change_mode(NODE_MOTOR, SPEED_MODE); // see SPEED_MODE in innfos_can_functions.hpp
	motor.change_mode(NODE_MOTOR, CURRENT_MODE);

	sleep(DELAY_SEC); 
	
	printf("[%s] MOTOR READY... \n\n", __FILE__);

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	// Control variables:
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	int i; // all-purpose counter

	///////////////////////////////////////////////////////////////////////////
	// Time variables:  
	///////////////////////////////////////////////////////////////////////////

	double t_s          = 0.0;

	double dt_ms        = 1.0 * (*dt_ns)/NS_PER_MS;
	double dt_actual_ms = 0.0;

	double dt_s         = dt_ms/MS_PER_S;

	static struct timespec tic, toc;

	///////////////////////////////////////////////////////////////////////////
	// Safety limits:
	///////////////////////////////////////////////////////////////////////////

	const double THETA_RAD_MAX = 1.0;

	int FAULT_ON = 0;

	///////////////////////////////////////////////////////////////////////////
	// Trajectory variables:
	///////////////////////////////////////////////////////////////////////////
    
	//Angle trajectory
	double ampl_curr_ref = 0;  
    
	double theta_ref	= 0, theta_ref_prev = 0;
	double dt_theta_ref = 0, dt_theta_ref_prev = 0;
	double curr_ref		= 0;

	//Torque trajectory
	double torque_ref	 = 0;
	double dt_torque_ref = 0, dt_torque_ref_prev = 0;
 	double ddt_torque_ref = 0, ddt_torque_ref_pre = 0;

	double theta_sea_m    = 0, theta_sea_m_prev = 0;
	double dt_theta_sea_m = 0, dt_theta_sea_m_prev = 0;
	double deflection_sea_ref  = 0, deflection_sea_ref_prev = 0;
   	double ddt_deflection_sea_ref   = 0;
	double curr_m         = 0;
	double torque_sea     = 0; 
 	double dt_torque_sea  = 0, dt_torque_sea_prev = 0;
 	double torque_sea_FT  = 0; 

	// Test trajectory parameters: 
	double AMPL_THETA_RAD_REF	 =  0;
	double AMPL_TORQUE_Nm_REF    =  0;
	float  ampl_theta_rad_ref_fl = -1.0;
	float  ampl_torque_Nm_ref_fl = -1.0;

	// double CURR_AMP_REF          = 0; // open-loop step respose test; TODO: delete at a later date
	double FREQ_REF_HZ			 =  0;  
	float  freq_ref_hz_fl		 = -1.0;  
	double omega_ref			 =  0;

	double N_T					 =  3.0; // # time constants for exponential rise
	double T_EXP_R				 =  1.0; // exponential rise time
		
	double t_step_s		= 0.0;

	// Query trajectory parameters:
	while (ampl_theta_rad_ref_fl < 0) {
		printf("\n");
		printf("Enter angle amplitude (rad): "); 
		scanf("%f", &ampl_theta_rad_ref_fl);  
	}
	AMPL_THETA_RAD_REF = (double)ampl_theta_rad_ref_fl;

	while (ampl_torque_Nm_ref_fl < 0) {
		printf("\n");
		printf("Enter torque amplitude (Nm): "); 
		scanf("%f", &ampl_torque_Nm_ref_fl);  
	}
	AMPL_TORQUE_Nm_REF = (double)ampl_torque_Nm_ref_fl;

	while (freq_ref_hz_fl < 0) {
		printf("\n");
		printf("Enter trajectory frequency (Hz): "); 
		scanf("%f", &freq_ref_hz_fl);  
	}
	FREQ_REF_HZ = (double)freq_ref_hz_fl;
	omega_ref = 2*PI*FREQ_REF_HZ;

	// Display section:
	printf("\n");
	printf("AMPL_THETA_RAD_REF = [%3.3lf]\n", AMPL_THETA_RAD_REF);
	printf("AMPL_TORQUE_Nm_REF = [%3.3lf]\n", AMPL_TORQUE_Nm_REF);
	printf("FREQ_REF_HZ        = [%3.3lf]\n", FREQ_REF_HZ);
	printf("\n");

	///////////////////////////////////////////////////////////////////////////
	// Motor/SEA model:
	///////////////////////////////////////////////////////////////////////////
    // SEA parameters
	double Js = 2.3268e-5;  // inertia of the plate, plate should not be included
	double Jm = 3.06e-4;    // inertia of the motor, need identificaiton
	double bm = 0; // unknown
		
	double I_sea_io    = I_SEA_IO;
	double b_sea_io    = B_SEA_IO;
	double k_sea_io    = K_SEA_IO;
	double fric_sea_io = FRIC_SEA_IO;

   	double PRE_EXTENSION_LEGENTH = 0.0005; 
    double OFFSET_ANGLE          = 0; 
    double trq_coeff_actual = K_TAU_SEA; // was TRQ_COEFF_NM_PER_AMP_SPEC

	///////////////////////////////////////////////////////////////////////////
	// SEA torque computing parameters
	///////////////////////////////////////////////////////////////////////////
    double k_spring  =  11900;     // N/m
    // double r1        =  0.0268; // m, inner radius
    double r1        =  0.0285;    // m, inner radius
	// double r2        =  0.0508; // m, outer radius
	double r2        =  0.053;     // m, outer radius
	// double l0        =  0.024;  // r2 - r1; initia value of spring
	double l0        =  0.0245;    // r2 - r1; initia value of spring
	double l         =  0.0245;     

	///////////////////////////////////////////////////////////////////////////
	// FB control: basic variables
	///////////////////////////////////////////////////////////////////////////

	// Error variables:
	double err_theta  = 0;
	double err_torque = 0;

	// Commanded FB torques:
	double tau_fb_in_lcomp = 0;  
	double tau_fb_in_pidf  = 0;    
	double tau_fb_in       = 0;  

	///////////////////////////////////////////////////////////////////////////
	// FB control I: lead compensator
	///////////////////////////////////////////////////////////////////////////

	// Bandwidth 15 Hz, phase margin 40 deg (after after independent spring constant and K_tau ID using 
	// test_sys_id_innfos_motor_v3_ktau_kspring_given.m / find_params_imped_SEA_innfos_scaled_v2_k_spring_est()):
	double K_c_bas      =  97.5; // compensator gain  
	double z_c          =  94.2; // compensator zero
	double p_c          =  645; // compensator pole

	// Lead compensator transfer function coefficients (discrete time):
	const int ORD_LCOMP = 1;

	double B_lcomp[ORD_LCOMP + 1] = {0, 0};
	double A_lcomp[ORD_LCOMP + 1] = {0, 0};

	coeffs_tf_discr_pole_zero_tustin(B_lcomp, A_lcomp, K_c_bas, p_c, z_c, dt_s);

	// Auxiliary variables:
	double err_theta_prev = 0; // previous error
	double tau_fb_in_prev = 0; // previous torque command

	///////////////////////////////////////////////////////////////////////////
	// FB control II: PID/lopass control
	///////////////////////////////////////////////////////////////////////////
	// PID/lopass parameters: bandwidth 30 Hz, phase margin 50 deg
	double K_pidf   =  234.99;

	double z_pidf_1 =   22.8;  
	double z_pidf_2 =   18;  

	double p_pidf_1 =    0;  
	double p_pidf_2 =  510; 
 
	//  PID/lopass transfer function coefficients (discrete time):
	const int ORD_PIDF = 2;

	double B_pidf[ORD_PIDF + 1] = {0, 0, 0};
	double A_pidf[ORD_PIDF + 1] = {0, 0, 0};

	coeffs_tf_discr_2pole_2zero_tustin(B_pidf, A_pidf, K_pidf, z_pidf_1, z_pidf_2, p_pidf_1, p_pidf_2, dt_s);

	// Auxiliary variables:
	double err_theta_arr[ORD_PIDF + 1] = {0, 0, 0};
	double tau_fb_in_arr[ORD_PIDF + 1] = {0, 0, 0};

	// Display section:
	printf("\n");
	printf("K_pidf    = [%3.6lf]\n", K_pidf); 
	printf("z_pidf_1  = [%3.6lf]\n", z_pidf_1); 
	printf("z_pidf_2  = [%3.6lf]\n", z_pidf_2); 
	printf("p_pidf_1  = [%3.6lf]\n", p_pidf_1); 
	printf("p_pidf_2  = [%3.6lf]\n", p_pidf_2); 
	printf("\n"); 

	///////////////////////////////////////////////////////////////////////////
	// Direct PID torque control
	///////////////////////////////////////////////////////////////////////////
    const int ORD_dir_PID = 2;
	
	// PID parameters
	double K_dir_P = 20;
	double K_dir_D = 0.1;
	double K_dir_I = 100;

    // PID tansfer function with low-pass filter (continuous time)
	double F_LP_HZ   = 50; // low-pass cutoff frequency
    double w_lp_rad  = 2*PI*F_LP_HZ;  

	double K_dir_PID    = K_dir_D*w_lp_rad;
	double b_dir_PID_1	= K_dir_P / K_dir_D;
	double b_dir_PID_2	= K_dir_I / K_dir_D;

	double a_dir_PID_1	= w_lp_rad;
	double a_dir_PID_2	= 0;
 
	// DZ transfer function coefficients (discrete time):
	double B_dir_PID[ORD_dir_PID + 1] = {0, 0, 0};
	double A_dir_PID[ORD_dir_PID + 1] = {0, 0, 0};

	coeffs_tf_discr_numord2_denord2_tustin(B_dir_PID, A_dir_PID, K_dir_PID, b_dir_PID_1, b_dir_PID_2, a_dir_PID_1, a_dir_PID_2, dt_s); 

	// Commanded direct PID torque:
	double tau_dir_PID = 0;

	// sample array
	double error_torq_dir_PID_arr[ORD_dir_PID + 1]     = {0, 0, 0};
	double tau_dir_PID_arr[ORD_dir_PID + 1]            = {0, 0, 0};

	// Display section:
	printf("w_lp_rad  = [%3.6lf]\n", w_lp_rad);
	printf("\n");
	printf("K_dir_PID    = [%3.6lf]\n", K_dir_PID); 
	printf("b_dir_PID_1  = [%3.6lf]\n", b_dir_PID_1); 
	printf("b_dir_PID_2  = [%3.6lf]\n", b_dir_PID_2); 
	printf("a_dir_PID_1  = [%3.6lf]\n", a_dir_PID_1); 
	printf("a_dir_PID_2  = [%3.6lf]\n", a_dir_PID_2); 
	printf("\n");

	for (i = 0; i <= ORD_dir_PID; i++)
		printf("B_dir_PID[%d] = [%3.6lf]\n", i, B_dir_PID[i]);
	printf("\n");
	
	for (i = 0; i <= ORD_dir_PID; i++)
		printf("A_dir_PID[%d] = [%3.6lf]\n", i, A_dir_PID[i]); 
	printf("\n");
 

	///////////////////////////////////////////////////////////////////////////
	// Cascaded PI torque control: inner-loop velocity and outer-loop torque
	///////////////////////////////////////////////////////////////////////////
    const int ORD_Cas_PI = 2;
    
	// Inner-loop velocity control PI transfer function coefficients (continuous time):
    double K_pi_inner = 2; // k=4, z=119 is From Gabriel's simulation tuning results
	double p_pi_inner = 0;
	double z_pi_inner = 20; 

	// Outer-loop torque control PI transfer function coefficients (continuous time):
    double K_pi_outer = 4; // k=4, z=19.5 is from gabriel's simulation tuning results 
	double p_pi_outer = 0;
	double z_pi_outer = 19.5;

    // Cascaded torque-loop tansfer function
    double K_cas_torq   = K_pi_inner*K_pi_outer;
	double z_cas_torq_1 = z_pi_inner;
	double z_cas_torq_2 = z_pi_outer;
	double p_cas_torq_1 = p_pi_inner;
	double p_cas_torq_2 = p_pi_outer;

	// Cascaded velocity-loop transfer function
    double F_DIFF_HZ   = 100; // cutoff frequency of lowpass filter;
	double w_cas_diff  = 2*PI*F_DIFF_HZ;

	double K_cas_vel   = K_pi_inner*w_cas_diff;
   	double z_cas_vel_1 = z_pi_inner;
	double z_cas_vel_2 = 0;
	double p_cas_vel_1 = p_pi_inner;
	double p_cas_vel_2 = w_cas_diff; 

    // Cascaded PI transfer function coefficients (discrete time):
	double B_cas_torq[ORD_Cas_PI + 1] = {0, 0, 0};
	double A_cas_torq[ORD_Cas_PI + 1] = {0, 0, 0};
	double B_cas_vel[ORD_Cas_PI + 1]  = {0, 0, 0};
	double A_cas_vel[ORD_Cas_PI + 1]  = {0, 0, 0};

	coeffs_tf_discr_2pole_2zero_tustin(B_cas_torq, A_cas_torq, K_cas_torq, z_cas_torq_1, z_cas_torq_2, p_cas_torq_1, p_cas_torq_2, dt_s);
	coeffs_tf_discr_2pole_2zero_tustin(B_cas_vel, A_cas_vel, K_cas_vel, z_cas_vel_1, z_cas_vel_2, p_cas_vel_1, p_cas_vel_2, dt_s);

	// Commanded cascaded PI torque:
	double tau_cas_pi_torq = 0;
	double tau_cas_pi_vel  = 0;
	double tau_cas_pi_in   = 0;

	// sample array
	double error_torq_arr[ORD_Cas_PI + 1] = {0, 0, 0};
	double theta_sea_m_cas_arr[ORD_Cas_PI + 1] = {0, 0, 0};
	double tau_cas_pi_torq_arr[ORD_Cas_PI + 1]   = {0, 0, 0};
	double tau_cas_pi_vel_arr[ORD_Cas_PI + 1]    = {0, 0, 0};

	// Display section:
	printf("w_cas_diff = [%3.6lf]\n", w_cas_diff);
	printf("\n");
	printf("K_pi_inner  = [%3.6lf]\n", K_pi_inner);
	printf("p_pi_inner  = [%3.6lf]\n", p_pi_inner);
	printf("z_pi_inner  = [%3.6lf]\n", z_pi_inner);
	printf("K_pi_outer  = [%3.6lf]\n", K_pi_outer);
	printf("p_pi_outer  = [%3.6lf]\n", p_pi_outer);
	printf("z_pi_outer  = [%3.6lf]\n", z_pi_outer);
	printf("\n");

	for (i = 0; i <= ORD_Cas_PI; i++)
		printf("B_cas_torq[%d] = [%3.6lf]\n", i, B_cas_torq[i]);
	printf("\n");
	
	for (i = 0; i <= ORD_Cas_PI; i++)
		printf("A_cas_torq[%d] = [%3.6lf]\n", i, A_cas_torq[i]); 
	printf("\n");

	for (i = 0; i <= ORD_Cas_PI; i++)
		printf("B_cas_vel[%d] = [%3.6lf]\n", i, B_cas_vel[i]);
	printf("\n");
	
	for (i = 0; i <= ORD_Cas_PI; i++)
		printf("A_cas_vel[%d] = [%3.6lf]\n", i, A_cas_vel[i]); 
	printf("\n");


	///////////////////////////////////////////////////////////////////////////
	// FF control: realizable "derivative impedance"
	///////////////////////////////////////////////////////////////////////////

	const int ORD_DZ = 2;

	// DZ transfer function coefficients (continuous time):
	double F_REAL_DZ_HZ  = 80.0; // DZ realizability filter cutoff frequency
	double w_real_DZ     = 2*PI*F_REAL_DZ_HZ;

	double K_DZ		= I_sea_io * w_real_DZ * w_real_DZ;
	double b_DZ_1	= b_sea_io / I_sea_io;
	double b_DZ_2	= k_sea_io / I_sea_io;

	double a_DZ_1	= 2*w_real_DZ;
	double a_DZ_2	= w_real_DZ*w_real_DZ;

	// DZ transfer function coefficients (discrete time):
	double B_DZ[ORD_DZ + 1] = {0, 0, 0};
	double A_DZ[ORD_DZ + 1] = {0, 0, 0};

	coeffs_tf_discr_numord2_denord2_tustin(B_DZ, A_DZ, K_DZ, b_DZ_1, b_DZ_2, a_DZ_1, a_DZ_2, dt_s); 

	// Commanded FF torque:
	double tau_ff_in = 0;

	// Derivative impedance sample arrays:
	double theta_ref_arr_DZ[ORD_DZ + 1] = {0, 0, 0};
	double tau_ff_in_arr[ORD_DZ + 1]    = {0, 0, 0};

	// Display section:
	printf("w_real_DZ  = [%3.6lf]\n", w_real_DZ);
	printf("\n");
	printf("K_DZ    = [%3.6lf]\n", K_DZ);
	printf("b_DZ_1  = [%3.6lf]\n", b_DZ_1);
	printf("b_DZ_2  = [%3.6lf]\n", b_DZ_2);
	printf("a_DZ_1  = [%3.6lf]\n", a_DZ_1);
	printf("a_DZ_2  = [%3.6lf]\n", a_DZ_2);
	printf("\n");

	for (i = 0; i <= ORD_DZ; i++)
		printf("B_DZ[%d] = [%3.6lf]\n", i, B_DZ[i]);
	printf("\n");
	
	for (i = 0; i <= ORD_DZ; i++)
		printf("A_DZ[%d] = [%3.6lf]\n", i, A_DZ[i]); 
	printf("\n");

	///////////////////////////////////////////////////////////////////////////
	// Coulomb friction parameters:	
	///////////////////////////////////////////////////////////////////////////

	double DT_THETA_TR = 0.5;  // Velocity thereshold of friction model
	double N_t         = 3.0;

	double tau_fric;

	///////////////////////////////////////////////////////////////////////////
	// Adaptive friction compensator:	
	///////////////////////////////////////////////////////////////////////////
    // compensator parameters
	double sigma_adap_fric       = 0; 
	double P_adap_fric           = 1;
	double lambda_adap_fric      = 1; 
	double err_region_adap_fric  = 0.001; 
	double Ts_adap_fric          = 0.001;

	double k_adap_fric           = 0;
	double k_adap_fric_prev      = 0;
	double dt_k_adap_fric_prev   = 0;
	double dt_k_adap_fric        = 0; 

	///////////////////////////////////////////////////////////////////////////
	// Disturbance observer (DOB):
	/////////////////////////////////////////////////////////////////////////// 

	const int ORD_DOB = 2; 

	double F_REAL_DOB_HZ  = 20.0; // DOB realizability filter cutoff frequency (see disturbance compensator)
	double w_real_inv_dob = 2*PI*F_REAL_DOB_HZ;
 
 	// Second-order filter for control input in DOB
    double K_filter_dob   = w_real_inv_dob * w_real_inv_dob;
	double p_filter_dob_1 = w_real_inv_dob;
	double p_filter_dob_2 = w_real_inv_dob;
 
 	double tau_cmd_in_arr_dob[ORD_DOB + 1] = {0, 0, 0};

	// DOB transfer function coefficients (continuous time) - realizable inverse of the motor's INTEGRAL ADMITTANCE: 
	double K_inv_dob	= I_sea_io * w_real_inv_dob * w_real_inv_dob;
	double b_inv_dob_1	= b_sea_io / I_sea_io;
	double b_inv_dob_2	= k_sea_io / I_sea_io;
	double a_inv_dob_1	= 2*w_real_inv_dob;
	double a_inv_dob_2	= w_real_inv_dob*w_real_inv_dob;

	double torque_sea_arr_dob[ORD_DOB + 1] = {0, 0, 0};

	// DZ transfer function coefficients (discrete time):
	double B_inv_dob[ORD_DOB + 1] = {0, 0, 0};
	double A_inv_dob[ORD_DOB + 1] = {0, 0, 0}; 
 	double B_filter_dob[ORD_DOB + 1] = {0, 0, 0};
	double A_filter_dob[ORD_DOB + 1] = {0, 0, 0}; 

	coeffs_tf_discr_numord2_denord2_tustin(B_inv_dob, A_inv_dob, K_inv_dob, b_inv_dob_1, b_inv_dob_2, a_inv_dob_1, a_inv_dob_2, dt_s); 
    coeffs_tf_discr_2pole_tustin(B_filter_dob, A_filter_dob, K_filter_dob, p_filter_dob_1, p_filter_dob_2, dt_s);
	
  // Auxiliary variables:
	double out_inv_dob = 0;
	double out_inv_dob_arr[ORD_DOB + 1] = {0, 0, 0};
    double tau_cmd_in_dob = 0; // filtered control input in DOB
	double tau_cmd_in_dob_arr[ORD_DOB + 1] = {0, 0, 0};  

	// Estimated disturbance torques:
	// double tau_dist_out = 0; // disturbance torque estimate (output shaft)
	double tau_dist_in = 0; // disturbance torque estimate (reflected at motor input)

	// Display section:
	for (i = 0; i <= ORD_DOB; i++)
		printf("B_inv_dob[%d] = [%3.6lf]\n", i, B_inv_dob[i]);
	printf("\n");
	
	for (i = 0; i <= ORD_DOB; i++)
		printf("A_inv_dob[%d] = [%3.6lf]\n", i, A_inv_dob[i]); 
	printf("\n");

	///////////////////////////////////////////////////////////////////////////
	// Disturbance compensator:
	///////////////////////////////////////////////////////////////////////////

	// Disturbance compensator transfer function coefficients (continuous time):
	double F_REAL_DCOMP_HZ  = 80.0; // Disturbance compensator realizability filter cutoff frequency
	double w_real_dcomp     = 2*PI*F_REAL_DCOMP_HZ;

	double K_dcomp   = w_real_dcomp;
	double p_dcomp_1 = w_real_dcomp;  
 
	// Disturbance compensator transfer function coefficients (discrete time):
	const int ORD_DCOMP = 1;

	double B_dcomp[ORD_DCOMP + 1] = {0, 0};
	double A_dcomp[ORD_DCOMP + 1] = {0, 0};

	coeffs_tf_discr_lopass_tustin(B_dcomp, A_dcomp, K_dcomp, p_dcomp_1, dt_s);

	// Auxiliary variables:
	double tau_dcomp = 0;  
	double tau_dist_in_prev = 0;
	double tau_dcomp_prev   = 0;

    ///////////////////////////////////////////////////////////////////////////
	// 1-order TDE-based MFC controller 
	///////////////////////////////////////////////////////////////////////////
    
	// Control para
	double alpha_TDEMFC = 2000000;
	double K_TDEMFC_p   = 400000;
	double K_TDEMFC_i   = 2000000;
    // continuous time
    double K_TDEMFC   = K_TDEMFC_p;
	double z_TDEMFC   = K_TDEMFC_i/K_TDEMFC_p;
	double p_TDEMFC   = 0;
    // discrete time
    const int ORD_TDEMFC = 1;

	double B_TDEMFC_PI[ORD_TDEMFC + 1] = {0, 0};
	double A_TDEMFC_PI[ORD_TDEMFC + 1] = {0, 0};
    
	coeffs_tf_discr_pole_zero_tustin(B_TDEMFC_PI, A_TDEMFC_PI, K_TDEMFC, p_TDEMFC, z_TDEMFC, dt_s);

  // First-order differentiator
	double F_DIFF_TDEMFC_HZ  = 50;
	double w_diff_TDEMFC     = 2*PI*F_DIFF_TDEMFC_HZ;
  // continuous time
	double K_diff_TDEMFC = w_diff_TDEMFC;
	double z_diff_TDEMFC = 0;
	double p_diff_TDEMFC = w_diff_TDEMFC;
  // discrete time
    const int ORD_TDEMFC_diff = 1;

	double B_TDEMFC_diff[ORD_TDEMFC_diff + 1] = {0, 0};
	double A_TDEMFC_diff[ORD_TDEMFC_diff + 1] = {0, 0};

	coeffs_tf_discr_pole_zero_tustin(B_TDEMFC_diff, A_TDEMFC_diff, K_diff_TDEMFC, p_diff_TDEMFC, z_diff_TDEMFC, dt_s);

  // Auxiliary variables:
	double tau_TDEMFC_in          = 0;
	double tau_TDEMFC_PI          = 0;
	double tau_TDEMFC_PI_prev     = 0; 
	double dis_TDE                = 0;  // TDE estimation result  
	double dis_TDE_prev           = 0; 
	double err_torque_sea_prev    = 0;
	double torque_sea_TDEMFC_prev = 0; 
	double torque_ref_TDEMFC_prev = 0;
	double tau_cmd_in_prev        = 0;

	// Display section:
	printf("w_diff_TDEMFC = [%3.6lf]\n", w_diff_TDEMFC);
	printf("\n");
	printf("z_TDEMFC  = [%3.6lf]\n", z_TDEMFC);
	printf("K_TDEMFC_p  = [%3.6lf]\n", K_TDEMFC_p);
	printf("K_TDEMFC_i  = [%3.6lf]\n", K_TDEMFC_i);
	printf("\n");

	///////////////////////////////////////////////////////////////////////////
	// Composite SMC controller to TDE-MFC 
	///////////////////////////////////////////////////////////////////////////
     double lemda_TDEMFCSMC   = 80;
     double eta_TDEMFCSMC     = 200000;   
     double epsilon_TDEMFCSMC = 2;  
    
	 double s_SMsurface       = 0;
	 double sat_SMsurface     = 0;

    ///////////////////////////////////////////////////////////////////////////
	// Feedback linearization-based SMC controller (FLSMC)
	///////////////////////////////////////////////////////////////////////////

	// Control parameters
     double alpha_FL     = 0;
	 double beta_FL      = 0;
	 double lambda_FLSMC = 10;
	 double xi_FLSMC     = 10000000;
	 double mu_FLSMC     = 0;
	 double s_FLSMC      = 0;
	 double sat_FLSMC    = 0;
     double epsilon_FLSMC = 10; 
	 double dh_dtheta_ord1 = 0.0; // 1-order patial differentiation
	 double dh_dtheta_ord2 = 0.0; // 2-order patial differentiation

	// 1-order Differentiator transfer function coefficients (discrete time):
	 double Fre_1ord_Diff_FLSMC_Hz  = 50; 
	 double w_1ord_Diff_FLSMC       = 2*PI*Fre_1ord_Diff_FLSMC_Hz;

	 double K_1ord_Diff_FLSMC	=  w_1ord_Diff_FLSMC;
	 double z_1ord_Diff_FLSMC  =  0  ;  
	 double p_1ord_Diff_FLSMC  =  w_1ord_Diff_FLSMC;   

	 const int ORD_1ord_Diff_FLSMC = 1;

	 double B_1ord_Diff_FLSMC[ORD_1ord_Diff_FLSMC + 1] = {0, 0};
	 double A_1ord_Diff_FLSMC[ORD_1ord_Diff_FLSMC + 1] = {0, 0};

	 coeffs_tf_discr_pole_zero_tustin(B_1ord_Diff_FLSMC, A_1ord_Diff_FLSMC, K_1ord_Diff_FLSMC, p_1ord_Diff_FLSMC, z_1ord_Diff_FLSMC, dt_s);

	// Auxiliary variables:
	 double theta_sea_prev = 0;   // previous theta_sea
	 double dt_theta_sea_prev = 0;  // previous theta_sea dt

	// 2-order Differentiator transfer function coefficients (discrete time):
	 double Fre_2ord_Diff_Hz  = 50; 
	 double w_2ord_Diff       = 2*PI*Fre_2ord_Diff_Hz;

	 double K_2ord_Diff =  w_2ord_Diff*w_2ord_Diff;

	 double z_2ord_Diff_1  =  0;  
	 double z_2ord_Diff_2  =  0;  

	 double p_2ord_Diff_1  =  w_2ord_Diff;  
	 double p_2ord_Diff_2  =  w_2ord_Diff; 
 
	 const int ORD_2ord_Diff = 2;

	 double B_2ord_Diff[ORD_2ord_Diff + 1] = {0, 0, 0};
	 double A_2ord_Diff[ORD_2ord_Diff + 1] = {0, 0, 0};

	coeffs_tf_discr_2pole_2zero_tustin(B_2ord_Diff, A_2ord_Diff, K_2ord_Diff, z_2ord_Diff_1, z_2ord_Diff_2, p_2ord_Diff_1, p_2ord_Diff_2, dt_s);
	
	// Auxiliary variables:
	 double torque_ref_arr[ORD_2ord_Diff + 1] = {0, 0, 0};
	 double ddt_torque_ref_arr[ORD_2ord_Diff + 1] = {0, 0, 0};
	 double theta_ref_arr[ORD_2ord_Diff + 1] = {0, 0, 0};
	 double ddt_theta_ref_arr[ORD_2ord_Diff + 1] = {0, 0, 0};
	 double tau_FLSMC_in = 0; 

	///////////////////////////////////////////////////////////////////////////
	// Torque-shaping
	///////////////////////////////////////////////////////////////////////////
     double J_bar = 4.5e-5;
	 double B_bar = 5.0e-3;
	 double J_sea_TorqShap = Js + 4*Jm;
	 double tau_cas_pi_TorqShaping_in = 0;

	///////////////////////////////////////////////////////////////////////////
	// Nonlinear disturbance observer-based controller (NDOBC)
	///////////////////////////////////////////////////////////////////////////

	 double gama_NDOB = 1000;      // bandwidth of the NDOB
     double J_sea_NDOB = 5e-4;     // J_sea = Js + 4*Jm, J_sea_NDOB need careful tunning, like control input gain

     double K_NDOB_z  = gama_NDOB;
	 double p_NDOB_z  = gama_NDOB;  
 
	 const int ORD_NDOB = 1;

	 double B_NDOB_z[ORD_NDOB + 1] = {0, 0};
	 double A_NDOB_z[ORD_NDOB + 1] = {0, 0};

	 coeffs_tf_discr_lopass_tustin(B_NDOB_z, A_NDOB_z, K_NDOB_z, p_NDOB_z, dt_s); 

	// Baseline nonlinear controller for NDOBC-PID term with low-pass filter (continuous time)
     double K_NDOBC_P = 10000;
	 double K_NDOBC_I = 0;
	 double K_NDOBC_D = 200;

	 double F_NDOBC_HZ   = 50; // low-pass cutoff frequency
     double w_NDOBC_rad  = 2*PI*F_NDOBC_HZ;  

	 double K_NDOBC_PID     = K_NDOBC_D*w_NDOBC_rad;
	 double b_NDOBC_PID_1	= K_NDOBC_P / K_NDOBC_D;
	 double b_NDOBC_PID_2	= K_NDOBC_I / K_NDOBC_D;

	 double a_NDOBC_PID_1	= w_NDOBC_rad;
	 double a_NDOBC_PID_2	= 0;
 
	// DZ transfer function coefficients (discrete time):
     const int ORD_NDOBC_PID = 2;

	 double B_NDOBC_PID[ORD_NDOBC_PID + 1] = {0, 0, 0};
	 double A_NDOBC_PID[ORD_NDOBC_PID + 1] = {0, 0, 0};

	 coeffs_tf_discr_numord2_denord2_tustin(B_NDOBC_PID, A_NDOBC_PID, K_NDOBC_PID, b_NDOBC_PID_1, b_NDOBC_PID_2, a_NDOBC_PID_1, a_NDOBC_PID_2, dt_s); 

	// Commanded NDOBC torque:
	 double tau_NDOBC_PID = 0;
     double tau_NDOBC_in  = 0;

	// Auxiliary variables
	 double error_theta_arr[ORD_NDOBC_PID + 1]   = {0, 0, 0};
	 double tau_NDOBC_PID_arr[ORD_NDOBC_PID + 1] = {0, 0, 0};
	 double deflection_sea_ref_arr[ORD_NDOBC_PID + 1] = {0, 0, 0};
	 double ddt_deflection_sea_ref_arr[ORD_NDOBC_PID + 1] = {0, 0, 0};
	 double u_NDOBC_in       = 0;
	 double pz_NDOB          = 0;
	 double z_NDOB           = 0;
	 double z_NDOB_prev      = 0;
	 double z_NDOB_lump      = 0;
	 double z_NDOB_lump_prev = 0;
	 double dis_NDOB         = 0; 

	///////////////////////////////////////////////////////////////////////////
	// From torque_ref to deflection_sea_ref
	///////////////////////////////////////////////////////////////////////////

	 double h_sea_prev           = 0;
	 double dh_dtheta_ord1_prev  = 0;

	///////////////////////////////////////////////////////////////////////////
	// Net control commands:
	///////////////////////////////////////////////////////////////////////////

	const int USE_PIDF  = 1;

	double tau_cmd_in   = 0;
	double tau_total_in = 0;

	///////////////////////////////////////////////////////////////////////////
	// Adjustable control factors:
	///////////////////////////////////////////////////////////////////////////

	const int USE_COMP_DIST = 0; // disturbance compensation switch, if friction comp, no linear DOB comp, vice versa

	// Feedback fraction:
	double frac_fb; // "feedback" fraction
	float  frac_fb_fl = -1.0;

	while (frac_fb_fl < 0) {
		printf("Enter feedback fraction (0..1): "); 
		scanf("%f", &frac_fb_fl);  
	}
	frac_fb = (double)frac_fb_fl;

	printf("\n");
	printf("frac_fb = [%3.3lf]\n",   frac_fb);
	printf("\n");


  	// Direct PID fraction:
	double frac_dir_PID; 
	float  frac_dir_PID_fl = -1.0;

	while (frac_dir_PID_fl < 0) {
		printf("Enter Direct PID fraction (0..1): "); 
		scanf("%f", &frac_dir_PID_fl);  
	}
	frac_dir_PID = (double)frac_dir_PID_fl;

	printf("\n");
	printf("frac_dir_PID = [%3.3lf]\n",  frac_dir_PID);
	printf("\n");

	// Cascaded PI fraction:
	double frac_cas_pi; 
	float  frac_cas_pi_fl = -1.0;

	while (frac_cas_pi_fl < 0) {
		printf("Enter cascaded PI fraction (0..1): "); 
		scanf("%f", &frac_cas_pi_fl);  
	}
	frac_cas_pi = (double)frac_cas_pi_fl;

	printf("\n");
	printf("frac_cas_pi = [%3.3lf]\n",   frac_cas_pi);
	printf("\n");

	// Feedforward fraction:
	double frac_ff; // 
	float  frac_ff_fl = -1.0;

	while (frac_ff_fl < 0) {
		printf("Enter feedforward fraction (0..1): "); 
		scanf("%f", &frac_ff_fl);  
	}
	frac_ff = (double)frac_ff_fl;

	printf("\n");
	printf("frac_ff = [%3.3lf]\n",   frac_ff);
	printf("\n");

	// Friction fraction:
	double frac_fric;  
	float  frac_fric_fl;

	if (USE_COMP_DIST) {
		frac_fric    = 0;
		frac_fric_fl = 0;
	}
	else {
		frac_fric_fl = -1.0;
		while (frac_fric_fl < 0) {
			printf("Enter Coulomb friction fraction (0..1): "); 
			scanf("%f", &frac_fric_fl);  
		}
		frac_fric = (double)frac_fric_fl;
	}

	printf("\n");
	printf("frac_fric = [%3.3lf]\n",   frac_fric);
	printf("\n");

	// Disturbance fraction:
	double frac_dist;  
	float  frac_dist_fl;

	if (USE_COMP_DIST) {
		frac_dist_fl = -1.0;
		while (frac_dist_fl < 0) {
			printf("Enter disturbance fraction (0..1): "); 
			scanf("%f", &frac_dist_fl);  
		}
		frac_dist = (double)frac_dist_fl;
	}
	else {
		frac_dist = 0;  
		frac_dist_fl = 0;
	}

	printf("\n");
	printf("frac_dist = [%3.3lf]\n",   frac_dist);
	printf("\n");
 
 	// 1-order TDEMFC fraction:
	double frac_TDEMFC; 
	float  frac_TDEMFC_fl = -1.0;

	while (frac_TDEMFC_fl < 0) {
		printf("Enter TDEMFC fraction (0..1): "); 
		scanf("%f", &frac_TDEMFC_fl);  
	}
	frac_TDEMFC = (double)frac_TDEMFC_fl;

	printf("\n");
	printf("frac_TDEMFC = [%3.3lf]\n",  frac_TDEMFC);
	printf("\n");

	// FLSMC fraction:
	double frac_FLSMC; 
	float  frac_FLSMC_fl = -1.0;

	while (frac_FLSMC_fl < 0) {
		printf("Enter FLSMC fraction (0..1): "); 
		scanf("%f", &frac_FLSMC_fl);  
	}
	frac_FLSMC = (double)frac_FLSMC_fl;

	printf("\n");
	printf("frac_FLSMC = [%3.3lf]\n",  frac_FLSMC);
	printf("\n");

	// NDOBC fraction:
	double frac_NDOBC; 
	float  frac_NDOBC_fl = -1.0;

	while (frac_NDOBC_fl < 0) {
		printf("Enter NDOBC fraction (0..1): "); 
		scanf("%f", &frac_NDOBC_fl);  
	}
	frac_NDOBC = (double)frac_NDOBC_fl;

	printf("\n");
	printf("frac_NDOBC = [%3.3lf]\n",  frac_NDOBC);
	printf("\n");
 
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	// Display variables:
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	int  T_DISP_MS = 1000;
	int  N_CYCLES_DISP   = T_DISP_MS/dt_ms;

	printf("[%s] N_CYCLES_DISP = [%d], dt_ms = [%3.1lf], dt_s = [%3.6lf]\n\n",  __FILE__, N_CYCLES_DISP, dt_ms, dt_s);

	///////////////////////////////////////////////////////////////////////////
	// Save PARAMETERS file
	///////////////////////////////////////////////////////////////////////////

	WRITE_PAR_SYS(param_file_id)
	fclose(param_file_id);

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	// Execute periodic task:
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	// Initial conditions:
	long cycle_count  = 0;
	run_on = 1;    

	double theta_sea_m_o = 1.0/PULLEY_SEA_RATIO*read_pos_innfos(&motor, NODE_MOTOR);
	
	// Launch task:
	periodic_task_init(&pinfo, *dt_ns); 

    while (run_on) {

		///////////////////////////////////////////////////////////////////////////
		// Initial time:
		///////////////////////////////////////////////////////////////////////////

        clock_gettime(CLOCK_MONOTONIC, &tic);

		///////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////
		// SENSORS AND FEEDBACK:
		///////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////
		// Innfos feedback or encoder feedback:
		///////////////////////////////////////////////////////////////////////////

		theta_sea_m		= 1.0/PULLEY_SEA_RATIO*read_pos_innfos(&motor, NODE_MOTOR) - theta_sea_m_o;
		dt_theta_sea_m	= 1.0/PULLEY_SEA_RATIO*read_vel_innfos(&motor, NODE_MOTOR); 
		// tf_discr_io_ord1(&dt_theta_sea_m, B_TDEMFC_diff, A_TDEMFC_diff, theta_sea_m, &theta_sea_m_prev, &dt_theta_sea_m_prev); 
               l        = sqrt(r1*r1 + r2*r2 - 2*r1*r2*(cos(theta_sea_m)));
		// torque_sea      = 12*k_spring*r1*r2*(1-(double)l0/l)*sin(theta_sea_m); 
        torque_sea      =  tau_est_SEA_model(PRE_EXTENSION_LEGENTH, OFFSET_ANGLE, theta_sea_m);  // little different from mine result above

		curr_m		= motor.read_cur_setpoint(NODE_MOTOR);  

		///////////////////////////////////////////////////////////////////////////
		// Safety conditions:
		///////////////////////////////////////////////////////////////////////////

		if (abs(curr_m) > CURR_AMP_MAX) { 
			printf("\n[%s] CURRENT limit exceeded \n", __FILE__); 
			FAULT_ON = 1;
		}
		else if (abs(theta_sea_m) > THETA_RAD_MAX) {
			printf("\n[%s] MOTOR ANGLE limit exceeded \n", __FILE__); 
			FAULT_ON = 1;
		}

		if (FAULT_ON) {
			motor.set_cur_setpoint(NODE_MOTOR, 0); 	
			motor.disable_motor(NODE_MOTOR);
			return NULL;
		}
	
		///////////////////////////////////////////////////////////////////////////
		// FT_data sensor readings:
		///////////////////////////////////////////////////////////////////////////

		if (FT_INPUT_ON) {
			// Read ADC:
			s526_adc_read(ADC_CHAN, NUM_ADC_CHAN, adc_data);

			// Convert into forces and torques:
			for (int ft_i = 0; ft_i < NUM_ADC_CHAN; ft_i++)
				adc_data_fl[ft_i] = (float) adc_data[ft_i];

			convert_adc_to_ft(adc_data_fl, FT_data);
		}
   		torque_sea_FT = adc_data_fl[5]; 
     
     
		///////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////
		// CONTROL COMPUTATIONS:
		///////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////
		// Reference trajectory generation:
		///////////////////////////////////////////////////////////////////////////

		if (t_s >= t_step_s) {
			// theta_ref = AMPL_THETA_RAD_REF;	
			theta_ref = AMPL_THETA_RAD_REF*(1.0 - exp( -N_T*(t_s - t_step_s)/T_EXP_R) ) * sin(omega_ref*(t_s - t_step_s));
		}
		else
			theta_ref  = 0;
		tf_discr_io_ord1(&dt_theta_ref, B_TDEMFC_diff, A_TDEMFC_diff, theta_ref, &theta_ref_prev, &dt_theta_ref_prev); 

		if (t_s >= t_step_s) {
		   // torque_ref = tau_est_SEA_model(PRE_EXTENSION_LEGENTH, OFFSET_ANGLE, theta_ref);  
		   // torque_ref = AMPL_TORQUE_Nm_REF*(1.0 - exp( -N_T*(t_s - t_step_s)/T_EXP_R) ); // step reference
		   torque_ref = AMPL_TORQUE_Nm_REF*(1.0 - exp( -N_T*(t_s - t_step_s)/T_EXP_R) ) * sin(omega_ref*(t_s - t_step_s));
		}
		else
			torque_ref = 0;
		tf_discr_io_ord1(&dt_torque_ref, B_TDEMFC_diff, A_TDEMFC_diff, torque_ref, &torque_ref_TDEMFC_prev, &dt_torque_ref_prev); 

		// frome torque_ref to deflection_sea_ref
		 deflection_sea_ref_prev =  deflection_sea_ref;
		 h_sea_prev          =  tau_est_SEA_model(PRE_EXTENSION_LEGENTH, OFFSET_ANGLE, deflection_sea_ref_prev);
         dh_dtheta_ord1_prev =  diff_h_theta_ord1(PRE_EXTENSION_LEGENTH, OFFSET_ANGLE, deflection_sea_ref_prev);
       
		 deflection_Newton(&deflection_sea_ref, h_sea_prev, torque_ref, dh_dtheta_ord1_prev, deflection_sea_ref_prev);

		///////////////////////////////////////////////////////////////////////////
		// Friction compensator:
		///////////////////////////////////////////////////////////////////////////	

		// torque_fric_coul(&tau_fric, fric_sea_io, dt_torque_ref, N_t, DT_THETA_TR); // Columb friction comp for initial position  
        if (dt_theta_sea_m > 0)
		    sigma_adap_fric =  1;
		else if (dt_theta_sea_m = 0 && tau_cmd_in > 0)
		    sigma_adap_fric = 1;
		else if (dt_theta_sea_m < 0)
		    sigma_adap_fric = -1;
		else if (dt_theta_sea_m = 0 && tau_cmd_in < 0)
		    sigma_adap_fric = -1;
		else
		    sigma_adap_fric = 0;	

		if (dt_theta_ref = 0 && fabs(err_theta) < err_region_adap_fric)
		    dt_k_adap_fric = 0;
		else
		    dt_k_adap_fric = P_adap_fric*sigma_adap_fric*(lambda_adap_fric*err_theta + dt_theta_ref - dt_theta_sea_m);
		
		k_adap_fric = k_adap_fric_prev + dt_k_adap_fric_prev*Ts_adap_fric/2 + dt_k_adap_fric*Ts_adap_fric/2;
		tau_fric = k_adap_fric*sigma_adap_fric; 
		// update
		k_adap_fric_prev    = k_adap_fric;
		dt_k_adap_fric_prev = dt_k_adap_fric;		
		
		///////////////////////////////////////////////////////////////////////////
		// Disturbance compensation with linear DOB:
		///////////////////////////////////////////////////////////////////////////	

		tf_discr_io_ord1(&tau_dcomp, B_dcomp, A_dcomp, tau_dist_in, &tau_dist_in_prev, &tau_dcomp_prev);

		///////////////////////////////////////////////////////////////////////////
		// Disturbance estimate:
		///////////////////////////////////////////////////////////////////////////	

		// tf_discr_io_help(&out_inv_dob, B_inv_dob, A_inv_dob, dt_theta_sea_m, dt_theta_sea_m_arr_dob, out_inv_dob_arr, ORD_DOB);
		tf_discr_io_help(&out_inv_dob, B_inv_dob, A_inv_dob, torque_sea, torque_sea_arr_dob, out_inv_dob_arr, ORD_DOB);
		tf_discr_io_help(&tau_cmd_in_dob, B_filter_dob, A_filter_dob, tau_cmd_in, tau_cmd_in_arr_dob, tau_cmd_in_dob_arr, ORD_DOB);
		tau_dist_in = -tau_cmd_in_dob + out_inv_dob; 

		///////////////////////////////////////////////////////////////////////////
		// Control commands:
		///////////////////////////////////////////////////////////////////////////
	
		err_theta  = deflection_sea_ref  - theta_sea_m;
		err_torque = torque_ref - torque_sea;		

	    // FB trajectory control command:
		
		tf_discr_io_ord1(&tau_fb_in_lcomp, B_lcomp, A_lcomp, err_theta, &err_theta_prev, &tau_fb_in_prev); 
		tf_discr_io_help(&tau_fb_in_pidf,  B_pidf,  A_pidf,  err_theta, err_theta_arr,   tau_fb_in_arr, ORD_PIDF);

		if (USE_PIDF)
			tau_fb_in = tau_fb_in_pidf;
		else
			tau_fb_in = tau_fb_in_lcomp;

	   // FF trajectory control command:
	   tf_discr_io_help(&tau_ff_in, B_DZ, A_DZ, theta_ref, theta_ref_arr_DZ, tau_ff_in_arr, ORD_DZ);

       // Direct PID torque controller
	   tf_discr_io_help(&tau_dir_PID, B_dir_PID, A_dir_PID, err_torque, error_torq_dir_PID_arr, tau_dir_PID_arr, ORD_dir_PID);
	   double tau_PID_torq = tau_dir_PID;

      // Cascaded PI control command
      tf_discr_io_help(&tau_cas_pi_torq, B_cas_torq, A_cas_torq, err_torque, error_torq_arr, tau_cas_pi_torq_arr, ORD_Cas_PI);
	  tf_discr_io_help(&tau_cas_pi_vel, B_cas_vel, A_cas_vel, theta_sea_m, theta_sea_m_cas_arr, tau_cas_pi_vel_arr, ORD_Cas_PI);
      tau_cas_pi_in = tau_cas_pi_torq - tau_cas_pi_vel;  

	  // Cascaded PI control with torque-shaping method
	  tau_cas_pi_TorqShaping_in = 0.5*(J_sea_TorqShap*frac_cas_pi*tau_cas_pi_in/J_bar + torque_sea -J_sea_TorqShap*torque_sea/J_bar - B_bar*J_sea_TorqShap*dt_torque_sea/J_bar);

      // 1-order TDEMFC control command
      tf_discr_io_ord1(&tau_TDEMFC_PI, B_TDEMFC_PI, A_TDEMFC_PI, err_torque, &err_torque_sea_prev, &tau_TDEMFC_PI_prev); 
      tf_discr_io_ord1(&dt_torque_sea, B_TDEMFC_diff, A_TDEMFC_diff, torque_sea, &torque_sea_TDEMFC_prev, &dt_torque_sea_prev); 
      TDE_ord1(&dis_TDE, alpha_TDEMFC, dt_torque_sea, tau_cmd_in, &dt_torque_sea_prev, &tau_cmd_in_prev);
	  // tau_TDEMFC_in = (dt_torque_ref - dis_TDE + tau_TDEMFC_PI)/alpha_TDEMFC; // TDEMFC
      s_SMsurface = lemda_TDEMFCSMC*err_torque + dt_torque_ref - dt_torque_sea; 
	   	if (s_SMsurface > epsilon_TDEMFCSMC)
     		sat_SMsurface = 1;
		else if (s_SMsurface < - epsilon_TDEMFCSMC)
		{
			sat_SMsurface = - 1;
		}
	    else
		{
			sat_SMsurface = s_SMsurface / epsilon_TDEMFCSMC;
		}
		tau_TDEMFC_in = (dt_torque_ref - dis_TDE + lemda_TDEMFCSMC*err_torque + eta_TDEMFCSMC*sat_SMsurface)/alpha_TDEMFC; // TDEMFCSMC

     // Feedback linearization-based SMC 
        dh_dtheta_ord1 = diff_h_theta_ord1(PRE_EXTENSION_LEGENTH, OFFSET_ANGLE, theta_sea_m);
	    dh_dtheta_ord2 = diff_h_theta_ord2(PRE_EXTENSION_LEGENTH, OFFSET_ANGLE, theta_sea_m);

		alpha_FL = (double) dh_dtheta_ord2*pow(dt_theta_sea_m,2);
		// beta_FL  = (double) dh_dtheta_ord1*2/(Js + 4.0*Jm);
        beta_FL = 10000000;

		s_FLSMC = lambda_FLSMC*err_torque + (dt_torque_ref - dt_torque_sea) ; // sliding furface
		if (s_FLSMC > epsilon_FLSMC)
     	      sat_FLSMC = 1;
		else if (s_SMsurface < - epsilon_FLSMC)
		{
			sat_FLSMC = - 1;
		}
	    else
		{
			sat_FLSMC = s_FLSMC / epsilon_FLSMC;
		}
        
	 tf_discr_io_help(&ddt_torque_ref,  B_2ord_Diff,  A_2ord_Diff,  torque_ref, torque_ref_arr, ddt_torque_ref_arr, ORD_2ord_Diff);
     tau_FLSMC_in = (ddt_torque_ref - alpha_FL + 0.5*beta_FL*torque_sea + lambda_FLSMC*(dt_torque_ref - dt_torque_sea) + (xi_FLSMC + mu_FLSMC)*sat_FLSMC)/beta_FL;

     // NDOBC
     tf_discr_io_ord1(&dt_theta_sea_m, B_TDEMFC_diff, A_TDEMFC_diff, theta_sea_m, &theta_sea_m_prev, &dt_theta_sea_m_prev); 
     pz_NDOB      = gama_NDOB*J_sea_NDOB*dt_theta_sea_m;
     z_NDOB_lump  = torque_sea - u_NDOBC_in - pz_NDOB;
     
	 tf_discr_io_ord1(&z_NDOB, B_NDOB_z, A_NDOB_z, z_NDOB_lump, &z_NDOB_lump_prev, &z_NDOB_prev); 
     dis_NDOB     = pz_NDOB + z_NDOB; 

     tf_discr_io_help(&ddt_deflection_sea_ref,  B_2ord_Diff,  A_2ord_Diff,  deflection_sea_ref, deflection_sea_ref_arr, ddt_deflection_sea_ref_arr, ORD_2ord_Diff);
     tf_discr_io_help(&tau_NDOBC_PID, B_NDOBC_PID, A_NDOBC_PID, err_theta, error_theta_arr, tau_NDOBC_PID_arr, ORD_NDOBC_PID);
	 u_NDOBC_in   = torque_sea - dis_NDOB + J_sea_NDOB*(ddt_deflection_sea_ref + tau_NDOBC_PID); 
	 
	 tau_NDOBC_in = 0.5*u_NDOBC_in;  

	// Net control command:
	// tau_cmd_in   =              frac_fb*tau_fb_in  + frac_ff*tau_ff_in; 
	// tau_total_in = tau_cmd_in - frac_fric*tau_fric - frac_dist*tau_dcomp;
     tau_cmd_in   = frac_dir_PID*tau_PID_torq - frac_dist*tau_dist_in + frac_cas_pi*tau_cas_pi_in 
	              + frac_TDEMFC*tau_TDEMFC_in  + frac_FLSMC*tau_FLSMC_in + frac_NDOBC*tau_NDOBC_in;
	 tau_total_in = tau_cmd_in + frac_fric*tau_fric; 

    // Send current command (including compensation torques):
    // CURR_AMP_REF = AMPL_THETA_RAD_REF; // open-loop step respose test; TODO: delete at a later date
	// curr_ref = CURR_AMP_REF*(1.0 - exp( -N_T*(t_s - t_step_s)/T_EXP_R/2.0) );

   		if (tau_total_in > 3)  // output saturation
		    curr_ref = 5;
			else if (tau_total_in < -3)
			{
				curr_ref = -5;
			}
			else
			{
				curr_ref = tau_total_in / trq_coeff_actual;
			}
		
		if (run_on > 0) 
			motor.set_cur_setpoint(NODE_MOTOR, curr_ref); 
		else 
			motor.set_cur_setpoint(NODE_MOTOR, 0); 

		///////////////////////////////////////////////////////////////////////////
		// Display section:
		///////////////////////////////////////////////////////////////////////////

		if ((cycle_count % N_CYCLES_DISP) == 0) {
			printf("t_s   = [%3.4lf]  theta_ref = [%3.4lf]  deflection_sea_ref = [%3.4lf] theta_sea_m  = [%3.4lf]  \n", t_s, theta_ref, deflection_sea_ref, theta_sea_m);
			printf("torque_ref = [%3.4lf]  torque_sea  = [%3.4lf]  torque_sea_FT  = [%3.4lf]  \n", torque_ref, torque_sea, torque_sea_FT);
            printf("curr_ref = [%3.4lf]\n", curr_ref); 
			printf("\n"); 
			// printf("M_z = [%3.6lf], tau_dist_OUT = [%3.6lf]\n\n", (double)FT_data[5]);
		}

		///////////////////////////////////////////////////////////////////////////
		// Update timer:		
		///////////////////////////////////////////////////////////////////////////

		cycle_count++;
		t_s  += 1.0*dt_ms/MS_PER_S;

		///////////////////////////////////////////////////////////////////////////
		// Compute time elapsed:
		///////////////////////////////////////////////////////////////////////////

		// Get the time again
		clock_gettime(CLOCK_MONOTONIC, &toc);
		dt_actual_ms = (double)(toc.tv_nsec - tic.tv_nsec)/NS_PER_MS;  

		///////////////////////////////////////////////////////////////////////////
		// Log data:		
		///////////////////////////////////////////////////////////////////////////

		WRITE_DAT_SYS(async_log_dat)

		///////////////////////////////////////////////////////////////////////////
		// Wait out rest of period:
		///////////////////////////////////////////////////////////////////////////

        wait_rest_of_period(&pinfo);
    }

	///////////////////////////////////////////////////////////////////////////
	// Exit procedures:
	///////////////////////////////////////////////////////////////////////////

	motor.set_cur_setpoint(NODE_MOTOR, 0); 
	
	motor.disable_motor(NODE_MOTOR);

	printf("[%s] EXIT \n", __FILE__);  

	return NULL;
}

