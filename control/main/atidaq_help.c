///////////////////////////////////////////////////////////////////////////                                                                     
// atidaq_help.c
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


#include "atidaq_help.h"

static Calibration* cal;

int
init_ft_sensor_ati(char* cal_file, int32_t* adc_chan_arr, int num_adc_chan) {

	 // create Calibration struct:
	printf("Initial force function come to here !!!!!!!!!!!\n");

	const int N_FT_OUT  = 6;

	double adc_data[N_FT_OUT];
	float  adc_data_fl[N_FT_OUT + 1];

    unsigned short index = 1; // index of calibration in file (second parameter; default  =  1)
    short sts;              // return value from functions

    // create Calibration struct:
	printf("\ncal_file = [%s]\n\n", cal_file);

	cal = createCalibration(cal_file, index);
	if (cal == NULL) {
		printf("\nSpecified calibration could not be loaded.\n");
		return -1;
	}
	
	// Set force units.
	// This step is optional; by default, the units are inherited from the calibration file.
	sts = SetForceUnits(cal,"N");
	switch (sts) {
		case 0: break;	// successful completion
		case 1: printf("Invalid Calibration struct"); return -1;
		case 2: printf("Invalid force units"); return -1;
		default: printf("Unknown error"); return -1;
	}

	// Set torque units.
	// This step is optional; by default, the units are inherited from the calibration file.
	sts = SetTorqueUnits(cal,"N-m");
	switch (sts) {
		case 0: break;	// successful completion
		case 1: printf("Invalid Calibration struct"); return -1;
		case 2: printf("Invalid torque units"); return -1;
		default: printf("Unknown error"); return -1;
	}

	// Read ADC:
	s526_adc_read(adc_chan_arr, num_adc_chan, adc_data, S526_DEFAULT_ADDRESS); 

	// Convert into forces and torques:
	for (int ft_i  =  0; ft_i < num_adc_chan; ft_i++) {
		adc_data_fl[ft_i]  =  (float) adc_data[ft_i];
	}

    // Calibrate:
    Bias(cal, adc_data_fl);

	// Display calibration info:
	printCalInfo(cal);

	// Verification (delete at a later date):
	/*
	double FT_data[6];
	float* bias_vector = cal->rt.bias_vector;

	ConvertToFT(cal, adc_data_fl, FT_data);

	printf("\n");
	printf("bias_vector: \n");
	for (int i = 0; i < N_FT_OUT; i++) {
		printf("%f\n", bias_vector[i]);
	}
	*/

	return 0;
}

void 
convert_adc_to_ft(float *adc_data_fl, float *FT_data) {
    ConvertToFT(cal,  adc_data_fl, FT_data);
}