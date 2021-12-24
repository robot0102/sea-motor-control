#ifndef ATIDAQ_FUNCS_H
#define ATIDAQ_FUNCS_H

#include <stddef.h>
#include "ftconfig.h"

void atidaq_init_converter(char *calib_file, char *force_units, 
                           char *torque_units);
                           
void atidaq_convert_to_ft(float *sample_reading, float *ft_array);

void atidaq_set_bias(float *bias_reading);

#endif