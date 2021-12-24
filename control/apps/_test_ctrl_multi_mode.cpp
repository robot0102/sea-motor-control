///////////////////////////////////////////////////////////////////////////                                                                     
// _test_ctrl_multi_mode.cpp
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
// extern int ctrl_mode;

void*
test_ctrl_multi_mode(void* dt_ns_ref) {

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

	// console_log->info("Generated data file");

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
	printf("ctrl_mode = [%d]\n", ctrl_mode);
	printf("\n");
	printf("test_ctrl_multi_mode(): Enabling motor... \n\n");

    motor.enable_motor(NODE_MOTOR);
	sleep(DELAY_SEC); 
	
	printf("test_ctrl_multi_mode(): changing motor mode... \n\n");

	// Initialize INNFOS mode (CRITICAL):
	if (ctrl_mode == MODE_VEL)
		motor.change_mode(NODE_MOTOR, SPEED_MODE); // see SPEED_MODE in innfos_can_functions.hpp

	else if (ctrl_mode == MODE_CURR)
		motor.change_mode(NODE_MOTOR, CURRENT_MODE); // see CURRENT_MODE in innfos_can_functions.hpp

	else if (ctrl_mode == MODE_P_CTRL)
		motor.change_mode(NODE_MOTOR, CURRENT_MODE); // see CURRENT_MODE in innfos_can_functions.hpp

	else {
		printf("[%s] Invalid ctrl_mode value...\n\n", __FILE__);
		exit(0);
	}
	sleep(DELAY_SEC); 
	
	printf("test_ctrl_multi_mode(): MOTOR READY... \n\n");

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	// Control variables:
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	// Trajectory variables:
	double AMPL_THETA_RAD_REF	= 0;
	double FREQ_REF_HZ			= 0;    

	double ampl_curr_ref		= 0;  

	double theta_ref	= 0;
	double dt_theta_ref	= 0;
	double curr_ref		= 0;

	double theta_m		= 0;
	double dt_theta_m	= 0;
    double curr_m		= 0;

	double K_p          = 0;
	double t_step_s		= 0;

	if (ctrl_mode == MODE_VEL)	{
		INIT_MODE_VEL
	}
	else if (ctrl_mode == MODE_CURR) {
		INIT_MODE_CURR
	}
	else if (ctrl_mode == MODE_P_CTRL) {
		INIT_MODE_P_CTRL
	}
	else {
		printf("\n[%s] Invalid ctrl_mode value...\n\n", __FILE__);  
		exit(0);
	}

	double omega_ref			= 2*PI*FREQ_REF_HZ;

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	// Time variables:
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	// double t_ms         = 0.0;
	double t_s          = 0.0;

	double dt_ms        = 1.0*(*dt_ns)/NS_PER_MS;
	double dt_actual_ms = 0.0;
	static struct timespec tic, toc;
 
 
 	///////////////////////////////////////////////////////////////////////////
	// SEA torque computing parameters
	///////////////////////////////////////////////////////////////////////////
    double k_spring  =  11900; // N/m
    double r1        =  0.0268; // m, inner radius
	double r2        =  0.0508; // m, outer radius
	double l0        =  0.024; // r2 - r1; initia value of spring
	double l         =  0.024; //   

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	// Display variables:
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	int  T_DISP_MS = 1000;
	int  N_CYCLES_DISP   = T_DISP_MS/dt_ms;

	printf("test_ctrl_multi_mode(): N_CYCLES_DISP = [%d], dt_ms = [%3.1lf], NS_PER_MS = [%ld]\n\n", N_CYCLES_DISP, dt_ms, NS_PER_MS);

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
    double theta_sea_m		= 0.5*theta_m;
 	        l               = sqrt(r1*r1 + r2*r2 - 2*r1*r2*(cos(theta_sea_m)));
    double 	torque_sea      = 12*k_spring*r1*r2*(1-(double)l0/l)*sin(theta_sea_m);

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
		// Motor control commands:
		///////////////////////////////////////////////////////////////////////////

		if (ctrl_mode == MODE_VEL) {
			CTRL_MODE_VEL
		}
		else if (ctrl_mode == MODE_CURR) {
			CTRL_MODE_CURR
		}
		else if (ctrl_mode == MODE_P_CTRL) {
			CTRL_MODE_P_CTRL
		}
		else {
			printf("\n[%s] Invalid ctrl_mode value...\n\n", __FILE__);
			exit(0);
		}

		///////////////////////////////////////////////////////////////////////////
		// Display section:
		///////////////////////////////////////////////////////////////////////////

		if ((cycle_count % N_CYCLES_DISP) == 0) {
			printf("t_s = [%3.3f], err_theta = [%3.3f], curr_ref = [%3.3f]:\n", t_s, theta_ref - theta_m, curr_ref);
			// printf("adc_data    =[%3.3f][%3.3f][%3.3f][%3.3f][%3.3f][%3.3f]\n",		adc_data[0],    adc_data[1],    adc_data[2],    adc_data[3],    adc_data[4],    adc_data[5]);
			// printf("adc_data_fl =[%3.3f][%3.3f][%3.3f][%3.3f][%3.3f][%3.3f]\n",		adc_data_fl[0], adc_data_fl[1], adc_data_fl[2], adc_data_fl[3], adc_data_fl[4], adc_data_fl[5]);
			printf("FT_data     =[%3.3f][%3.3f][%3.3f][%3.3f][%3.3f][%3.3f]\n\n",	FT_data[0],     FT_data[1],     FT_data[2],     FT_data[3],     FT_data[4],     FT_data[5]);
		}

		///////////////////////////////////////////////////////////////////////////
		// Update timer:		
		///////////////////////////////////////////////////////////////////////////

		cycle_count++;
		t_s  += 1.0*dt_ms/MS_PER_S;
		// t_ms += 1.0*dt_ms;

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

	if (ctrl_mode == MODE_VEL)
		set_vel_innfos(0, &motor, NODE_MOTOR);

	else if (ctrl_mode == MODE_CURR)
		motor.set_cur_setpoint(NODE_MOTOR, 0);

	else if (ctrl_mode == MODE_P_CTRL)
		motor.set_cur_setpoint(NODE_MOTOR, 0);

	else {
		printf("\n[%s] Invalid ctrl_mode value...\n\n", __FILE__);
		exit(0);
	}
	
	motor.disable_motor(NODE_MOTOR);

	printf("[%s] EXIT \n", __FILE__);  

	return NULL;
}


