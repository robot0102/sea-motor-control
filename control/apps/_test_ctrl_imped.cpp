///////////////////////////////////////////////////////////////////////////                                                                     
// _test_ctrl_imped.cpp
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

extern int run_on;

void*
test_ctrl_imped(void* dt_ns_ref) {

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

	double dt_ms        = 1.0*(*dt_ns)/NS_PER_MS;
	double dt_actual_ms = 0.0;

	double dt_s         = dt_ms/MS_PER_S; 

	static struct timespec tic, toc; 

	///////////////////////////////////////////////////////////////////////////
	// Trajectory variables:
	///////////////////////////////////////////////////////////////////////////

	double ampl_curr_ref		= 0;  

	double theta_ref	= 0;
	double dt_theta_ref = 0, dt_theta_ref_prev = 0;
	double curr_ref		= 0;

	double theta_m		= 0;
	double dt_theta_m	= 0;
    double curr_m		= 0;

	// Test trajectory parameters: 
	int TEST_TRAJ_TRACK_ON  = 1;

	double AMPL_THETA_RAD_REF	 = 0;
	float  ampl_theta_rad_ref_fl = -1.0;

	double FREQ_REF_HZ			= 1.0;  
	double omega_ref			= 2*PI*FREQ_REF_HZ;
	double N_T					= 3.0; // # time constants for exponential rise
	double T_EXP_R				= 1.0; // exponential rise time
		
	double t_step_s		= 0.0;

	if (TEST_TRAJ_TRACK_ON)
		while (ampl_theta_rad_ref_fl < 0) {
			printf("\n");
			printf("Enter angle amplitude (rad): "); 
			scanf("%f", &ampl_theta_rad_ref_fl);  
		}
	else
		ampl_theta_rad_ref_fl = 0.5;

	AMPL_THETA_RAD_REF = (double)ampl_theta_rad_ref_fl; 

	// Display section:
	printf("\n");
	printf("AMPL_THETA_RAD_REF = [%3.3lf]\n", AMPL_THETA_RAD_REF); 
	printf("\n");

	///////////////////////////////////////////////////////////////////////////
	// Motor model:
	///////////////////////////////////////////////////////////////////////////

	double trq_coeff_actual = TRQ_COEFF_NM_PER_AMP;

	// Motor TF order 1
	double K_io = 1.0/I_MOMT_IO;
	double p_io = B_DAMP_IO/I_MOMT_IO; 

	// Output admittance parameters: ACTUAL
	double I_out = GEAR_RATIO*I_MOMT_IO; 
	double b_out = GEAR_RATIO*B_DAMP_IO;

	///////////////////////////////////////////////////////////////////////////
	// FB control: position tracking
	///////////////////////////////////////////////////////////////////////////

	// Simple P control:
	double K_p = 0;

	// Lead compensator parameters: bandwidth 30 rad/s
	/*
	double z_c          =   6.3; // compensator zero
	double p_c          = 400.0; // compensator pole
	double K_c_bas      =  65.0; // compensator gain (basic)
	*/

	// Lead compensator parameters: bandwidth 40 rad/s
	double z_c          =   8.3; // compensator zero
	double p_c          = 537.0; // compensator pole
	double K_c_bas      = 116.0; // compensator gain (basic)

	// Lead compensator transfer function coefficients (discrete time):
	const int ORD_LCOMP = 1;

	double B_lcomp[ORD_LCOMP + 1] = {0, 0};
	double A_lcomp[ORD_LCOMP + 1] = {0, 0};

	coeffs_tf_discr_pole_zero_tustin(B_lcomp, A_lcomp, K_c_bas, p_c, z_c, dt_s);

	// Error variables:
	double err_theta = 0, err_theta_prev = 0;

	// Commanded FB torques:
	double tau_fb_in = 0, tau_fb_in_prev = 0; // motor torque command

	///////////////////////////////////////////////////////////////////////////
	// FF control: realizable "derivative impedance"
	///////////////////////////////////////////////////////////////////////////

	const int ORD_DZ = 2;

	// DZ transfer function coefficients (continuous time):
	double F_REAL_DZ_HZ  = 80.0; // DZ realizability filter cutoff frequency
	double w_real_DZ     = 2*PI*F_REAL_DZ_HZ;

	double K_DZ		= I_MOMT_IO*w_real_DZ*w_real_DZ;
	double z_DZ_1	= 0;
	double z_DZ_2	= B_DAMP_IO/I_MOMT_IO;
	double p_DZ_1	= w_real_DZ;
	double p_DZ_2	= w_real_DZ;

	// DZ transfer function coefficients (discrete time):
	double B_DZ[ORD_DZ + 1] = {0, 0, 0};
	double A_DZ[ORD_DZ + 1] = {0, 0, 0};

	coeffs_tf_discr_2pole_2zero_tustin(B_DZ, A_DZ, K_DZ, z_DZ_1, z_DZ_2, p_DZ_1, p_DZ_2, dt_s);

	// Commanded FF torque:
	double tau_ff_in = 0;
	
	// Derivative impedance sample arrays:
	double theta_ref_arr_DZ[ORD_DZ + 1] = {0, 0, 0};
	double tau_ff_in_arr[ORD_DZ + 1] = {0, 0, 0};

	// Display section:
	printf("w_real_DZ  = [%3.6lf]\n", w_real_DZ);
	printf("\n");
	printf("K_DZ    = [%3.6lf]\n", K_DZ);
	printf("z_DZ_1  = [%3.6lf]\n", z_DZ_1);
	printf("z_DZ_2  = [%3.6lf]\n", z_DZ_2);
	printf("p_DZ_1  = [%3.6lf]\n", p_DZ_1);
	printf("p_DZ_2  = [%3.6lf]\n", p_DZ_2);
	printf("\n");

	for (i = 0; i <= ORD_DZ; i++)
		printf("B_DZ[%d] = [%3.6lf]\n", i, B_DZ[i]);
	printf("\n");
	
	for (i = 0; i <= ORD_DZ; i++)
		printf("A_DZ[%d] = [%3.6lf]\n", i, A_DZ[i]); 
	printf("\n");

	///////////////////////////////////////////////////////////////////////////
	// Admittance model:
	///////////////////////////////////////////////////////////////////////////

	// Desired output admittance parameters: DESIRED
	double I_out_des;
	double b_out_des;

	// Compute desired inertia:
	double fact_I_des; // inertia factor
	float  fact_I_des_fl = -1.0;

	if ( !(TEST_TRAJ_TRACK_ON) )
		while (fact_I_des_fl <= 0) {
			printf("Enter inertia factor (>= 1): "); 
			scanf("%f", &fact_I_des_fl);  
		}
	else
		fact_I_des_fl = 1; // dummy value

	fact_I_des = (double)fact_I_des_fl;
	I_out_des  = fact_I_des*I_out;

	// Compute desired damping:
	double frac_b_des; // damping fraction
	float  frac_b_des_fl = 2.0;

	if ( !(TEST_TRAJ_TRACK_ON) )
		while (frac_b_des_fl > 1.0) {
			printf("Enter damping fraction (0..1): "); 
			scanf("%f", &frac_b_des_fl);  
		}
	else
		frac_b_des_fl = 0; // dummy value

	frac_b_des = (double)frac_b_des_fl;
	b_out_des  = frac_b_des*b_out;

	// Integral admittance transfer function coefficients (discrete time):
	const int ORD_INTY_DES = 2;

	double B_intY_des[ORD_INTY_DES + 1] = {0, 0, 0};
	double A_intY_des[ORD_INTY_DES + 1] = {0, 0, 0};

	coeffs_tf_discr_2pole_tustin(B_intY_des, A_intY_des, 1.0/I_out_des, 0, b_out_des/I_out_des, dt_s);

	// Integral admitance sample arrays:
	double tau_dist_out_arr[ORD_INTY_DES + 1] = {0, 0, 0};
	double theta_ref_arr[ORD_INTY_DES + 1]   = {0, 0, 0};

	// Display section:
	printf("\n");
	printf("I_out = [%3.6lf]\n", I_out);
	printf("b_out = [%3.6lf]\n", b_out);
	printf("\n");
	printf("fact_I_des  = [%3.3lf]\n", fact_I_des);
	printf("I_out_des   = [%3.6lf]\n", I_out_des);
	printf("\n");
	printf("frac_b_des  = [%3.3lf]\n", frac_b_des);
	printf("b_out_des   = [%3.6lf]\n", b_out_des);
	printf("\n");

	for (i = 0; i <= ORD_INTY_DES; i++)
		printf("B_intY_des[%d] = [%3.6lf]\n", i, B_intY_des[i]);
	printf("\n");
	
	for (i = 0; i <= ORD_INTY_DES; i++)
		printf("A_intY_des[%d] = [%3.6lf]\n", i, A_intY_des[i]); 
	printf("\n");

	///////////////////////////////////////////////////////////////////////////
	// Coulomb friction parameters:	
	///////////////////////////////////////////////////////////////////////////

	double DT_THETA_TR = 0.5;
	double N_t         = 3.0;

	double tau_fric;

	///////////////////////////////////////////////////////////////////////////
	// Disturbance observer (DOB):
	///////////////////////////////////////////////////////////////////////////

	const int ORD_DOB = 2;

	// DOB transfer function coefficients (continuous time) - realizable inverse of the motor's integral admittance:
	double K_inv_dob, z_inv_dob_1, z_inv_dob_2, p_inv_dob_1, p_inv_dob_2;

	double F_REAL_DOB_HZ  = 40.0; // DOB realizability filter cutoff frequency
	double w_real_inv_dob = 2*PI*F_REAL_DOB_HZ;

	coeffs_tf_cont_inv_intadmitt(&K_inv_dob, &z_inv_dob_1, &z_inv_dob_2, &p_inv_dob_1, &p_inv_dob_2, 
		K_io, p_io, w_real_inv_dob);

	// DOB transfer function coefficients (discrete time):
	double B_inv_dob[ORD_DOB + 1] = {0, 0, 0};
	double A_inv_dob[ORD_DOB + 1] = {0, 0, 0};

	coeffs_tf_discr_2pole_2zero_tustin(B_inv_dob, A_inv_dob, K_inv_dob, z_inv_dob_1, z_inv_dob_2, p_inv_dob_1, p_inv_dob_2, dt_s);

	// Estimated disturbance torques:
	double tau_dist_out; // disturbance torque estimate (output shaft)
	double tau_dist_in; // disturbance torque estimate (reflected at motor input)

	// Display section:
	printf("I_MOMT_IO = [%3.6lf]\n", I_MOMT_IO);
	printf("B_DAMP_IO = [%3.6lf]\n", B_DAMP_IO);
	printf("\n");
	printf("K_io  = [%3.6lf]\n", K_io);
	printf("p_io  = [%3.6lf]\n", p_io);
	printf("\n");
	printf("w_real_inv_dob  = [%3.6lf]\n", w_real_inv_dob);
	printf("\n");
	printf("K_inv_dob    = [%3.6lf]\n", K_inv_dob);
	printf("z_inv_dob_1  = [%3.6lf]\n", z_inv_dob_1);
	printf("z_inv_dob_2  = [%3.6lf]\n", z_inv_dob_2);
	printf("p_inv_dob_1  = [%3.6lf]\n", p_inv_dob_1);
	printf("p_inv_dob_2  = [%3.6lf]\n", p_inv_dob_2);
	printf("\n");

	for (i = 0; i <= ORD_DOB; i++)
		printf("B_inv_dob[%d] = [%3.6lf]\n", i, B_inv_dob[i]);
	printf("\n");
	
	for (i = 0; i <= ORD_DOB; i++)
		printf("A_inv_dob[%d] = [%3.6lf]\n", i, A_inv_dob[i]); 
	printf("\n");

	///////////////////////////////////////////////////////////////////////////
	// Net control commands:
	///////////////////////////////////////////////////////////////////////////

	double tau_total_in_cmd = 0;

	///////////////////////////////////////////////////////////////////////////
	// Adjustable control factors:
	///////////////////////////////////////////////////////////////////////////

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

	// Feedforward fraction:
	double frac_ff; // "feedback" fraction
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
	double frac_fric; //  
	float  frac_fric_fl = -1.0;

	while (frac_fric_fl < 0) {
		printf("Enter Coulomb friction fraction (0..1): "); 
		scanf("%f", &frac_fric_fl);  
	}
	frac_fric = (double)frac_fric_fl;;

	printf("\n");
	printf("frac_fric = [%3.3lf]\n",   frac_fric);
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

	double theta_m_o = read_pos_innfos(&motor, NODE_MOTOR);
	
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
		// Innfos feedback:
		///////////////////////////////////////////////////////////////////////////

		theta_m		= read_pos_innfos(&motor, NODE_MOTOR) - theta_m_o;
		dt_theta_m	= read_vel_innfos(&motor, NODE_MOTOR); 

		curr_m		= motor.read_cur_setpoint(NODE_MOTOR);  

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

		///////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////
		// CONTROL COMPUTATIONS:
		///////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////
		// Disturbance estimate:
		///////////////////////////////////////////////////////////////////////////	

		inv_dob_tf_discr_io(&tau_dist_in, tau_total_in_cmd, theta_m, B_inv_dob, A_inv_dob, ORD_DOB);

		tau_dist_out = GEAR_RATIO*tau_dist_in;

		/*
		printf("tau_dist_in = [%3.6lf]\n", tau_dist_in);
		printf("___________________________________________________________________\n");
		*/

		///////////////////////////////////////////////////////////////////////////
		// Coulomb friction estimate:
		///////////////////////////////////////////////////////////////////////////	

		torque_fric_coul(&tau_fric, FRIC_IO, dt_theta_m, N_t, DT_THETA_TR);

		///////////////////////////////////////////////////////////////////////////
		// Reference trajectory generation:
		///////////////////////////////////////////////////////////////////////////

		if ( !(TEST_TRAJ_TRACK_ON) ) {
			// Admittance control TF - I/O operation:
			tf_discr_io_help(&theta_ref, B_intY_des, A_intY_des, tau_dist_out, tau_dist_out_arr, theta_ref_arr, ORD_INTY_DES);
		}
		else {
			if (t_s >= t_step_s) {
				// theta_ref = AMPL_THETA_RAD_REF;	
				theta_ref = AMPL_THETA_RAD_REF*(1.0 - exp( -N_T*(t_s - t_step_s)/T_EXP_R) ) * sin(omega_ref*(t_s - t_step_s));
			}
			else
				theta_ref = 0;
		}

		///////////////////////////////////////////////////////////////////////////
		// Control commands:
		///////////////////////////////////////////////////////////////////////////

		// FB trajectory control command:
		err_theta = theta_ref - theta_m;
		tf_discr_io_ord1(&tau_fb_in, B_lcomp, A_lcomp, err_theta, &err_theta_prev, &tau_fb_in_prev); 

		// FF trajectory control command:
		tf_discr_io_help(&tau_ff_in, B_DZ, A_DZ, theta_ref, theta_ref_arr_DZ, tau_ff_in_arr, ORD_DZ);

		// Net control command:
		tau_total_in_cmd		= frac_fb*tau_fb_in + frac_ff*tau_ff_in; 

		// Send current command:
		curr_ref = (tau_total_in_cmd + frac_fric*tau_fric) / trq_coeff_actual; // NOTE: friction does not belong to the plant model
		
		if (run_on > 0) 
			motor.set_cur_setpoint(NODE_MOTOR, curr_ref); 
		else 
			motor.set_cur_setpoint(NODE_MOTOR, 0); 

		///////////////////////////////////////////////////////////////////////////
		// Display section:
		///////////////////////////////////////////////////////////////////////////

		if ((cycle_count % N_CYCLES_DISP) == 0) {
			printf("t_s       = [%3.4lf]  theta_ref = [%3.4lf]  theta_m  = [%3.4lf]  tau_dist_OUT = [%3.4lf]\n", t_s, theta_ref, theta_m, tau_dist_out);
		    printf("tau_fb_in = [%3.4lf]  tau_ff_in = [%3.4lf]  tau_fric = [%3.4lf]  curr_ref     = [%3.4lf]\n", tau_fb_in, tau_ff_in, tau_fric, curr_ref);
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


