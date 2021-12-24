///////////////////////////////////////////////////////////////////////////                                                                     
// atidaq_help.h
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


#include "ftconfig.h"
#include "model526.h"

// static char CAL_FILE[] = "calibration/FT22694.cal"; // calibration file for ATI FT sensor in use

// static char CAL_FILE[] = "calibration_mini45/FT30578.cal"; // calibration file for ATI FT sensor in use

static char CAL_FILE[] = "/home/com1/zhimin/2_dof_sea_test/calibration_mini45/FT30578.cal"; 

int 
init_ft_sensor_ati(char* cal_file, int32_t* adc_chan_arr, int num_adc_chan); 

void  
convert_adc_to_ft(float *adc_data_fl, float *FT_data); 