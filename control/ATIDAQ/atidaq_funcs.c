#include "atidaq_funcs.h"

static Calibration *cal;
static unsigned short index = 1;
static short sts;

void atidaq_init_converter(char *calib_file, char *force_units, char *torque_units) {
    
    // create Calibration struct
    cal=createCalibration(calib_file, index);
    if (cal == NULL) {
        printf("\nSpecified calibration could not be loaded.\n");
        //scanf(".");
        return 0;
    }

    // Set force units.
    // This step is optional; by default, the units are inherited from the calibration file.
    sts=SetForceUnits(cal, force_units);
    switch (sts) {
    case 0: break;  // successful completion
    case 1: printf("Invalid Calibration struct"); return 0;
    case 2: printf("Invalid force units"); return 0;
    default: printf("Unknown error"); return 0;
    }

    // Set torque units.
    // This step is optional; by default, the units are inherited from the calibration file.
    sts=SetTorqueUnits(cal, torque_units);
    switch (sts) {
    case 0: break;  // successful completion
    case 1: printf("Invalid Calibration struct"); return 0;
    case 2: printf("Invalid torque units"); return 0;
    default: printf("Unknown error"); return 0;
    }
}

void atidaq_convert_to_ft(float *sample_reading, float *ft_array) {
    ConvertToFT(cal, sample_reading, ft_array);
}

void atidaq_set_bias(float *bias_reading) {
    Bias(cal, bias_reading);
}