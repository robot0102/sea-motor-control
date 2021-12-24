#include <iostream>
#include <string> 

using namespace std;  

#include "mitcan.h"  
#include "_test_scripts_zhimin.hpp" 

// int float_to_uint(float x, float x_min, float x_max, int bits){
//     /// Converts a float to an unsigned int, given range and number of bits ///
//     float span = x_max - x_min; 
//     float offset = x_min; 
//     return (int) ((x-offset)*((float)((1<<bits)-1))/span); 
// }

int begin526(int * adc_channels, int dev_add)
{

    printf("Initializing Sensoray model_inputsModel 526...\n");
    printf("before s526_init\n"); 
    int iret = s526_init(dev_add); 

    printf("after s526_init\n");
    if(iret)
    {
        printf("DAQ Initialization failed with error %d: %s\n", iret, strerror(iret));
        return(0);
    }
    printf("Reading board id...");
    printf("before s526_read_id\n");

    iret = s526_read_id(dev_add);

    printf("after s526_read_id\n");
    if((iret == 0x526b) || (iret == 0x526a))
    {
        printf("Read correct board id: %04x\n", iret);
    }
    else
    {
        printf("Wrong board id %04x! Maybe the address you gave init() is wrong?\n", iret);
        return(0);
    }

    s526_dac_init(dev_add); 
    s526_adc_init(adc_channels,8,dev_add);

    return 0;
}

// int test_value; 
int main(){

    ///////////////////////////////////////////////////////////////////////////
	// Motor Settings and Test: 
	///////////////////////////////////////////////////////////////////////////

    // int test_p_des, test_v_des, test_kp, test_kd, test_torque; 
    
    // cout << "initial can" << endl; 
    // CANDevice can0((char *) "can0"); 
    // CANDevice can1((char *) "can1"); 
    // CANDevice can2((char *) "can2"); 
    // CANDevice can3((char *) "can3"); 

    // // CANDevice can1((char *) "can1"); 
    // can0.begin(); 
    // can1.begin(); 
    // can2.begin(); 
    // can3.begin(); 

    // Mitcan can_test(can0); 
    // Mitcan can_test_1(can1); 
    // Mitcan can_test_2(can2); 
    // Mitcan can_test_3(can3); 

    // cout << "Enable motor !!!" << endl;  

    // // Mitcan can1; 
    // can_test.enable_motor(); 
    // cout << "CAN 0" << endl; 

    // can_test_1.enable_motor(); 
    // cout << "CAN 1" << endl; 

    // // cout << "Disable motor !!!" << endl; 

    // // can_test_1.Set_Command(1, 0.5, 0., 5.0, 0.4, 0.); 
    // // cout << "Send position !!!" << endl; 

    // float pos_1, vel_1, cur_1; 
    // struct can_frame rframe_1 = can_test.readcan(&pos_1, &vel_1, &cur_1); 
    // cout << "pos_1" << pos_1 << endl; 
    // cout << "vel_1" << vel_1 << endl; 
    // cout << "cur_1" << cur_1 << endl; 

    // // cout << "Read CAN !!!" << endl; 
    // float pos_2, vel_2, cur_2; 
    // struct can_frame rframe_2 = can_test_1.readcan(&pos_2, &vel_2, &cur_2); 
    // cout << "pos_2" << pos_2 << endl; 
    // cout << "vel_2" << vel_2 << endl; 
    // cout << "cur_2" << cur_2 << endl; 

    // can_test.disable_motor(); 
    // can_test_1.disable_motor(); 
    // cout << "Disable motor !!!" << endl; 

    // ///////////////////////////////////////////////////////////////////////////
	// // Initialize Sensoray 526:
	// ///////////////////////////////////////////////////////////////////////////

    
    //  ADC channels;
    const int NUM_ADC_CHAN			= 6; 
    
    int32_t ADC_CHAN[NUM_ADC_CHAN]	= {0, 1, 2, 3, 4, 5};  
    // int32_t ADC_CHAN[NUM_ADC_CHAN]	= {7, 6, 5, 4, 3, 2, 1, 0};  

    double adc_data[NUM_ADC_CHAN]	= {0, 0, 0, 0, 0, 0};  

    int adc_channels[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    double adc_sample[8]= {0, 0, 0, 0, 0, 0, 0, 0};
    int current_count[5] = {0,0,0,0,0};
    double current_analog[8] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

    begin526(adc_channels, S526_DEFAULT_ADDRESS); // init dac & adc 1

    cout << "initial s526 !!!" << endl;   

    // // s526_read_id(); 

    // // cout << "initial DAC !!!" << endl; 
    
	// // Initialize hardware: 
	// s526_init(); 

    // cout << "initial DAC !!!" << endl; 
    
    // s526_adc_init(ADC_CHAN, NUM_ADC_CHAN); 

    // cout << "Test ADC read !!!" << endl; 

    // // Read ADC:
    // s526_adc_read(ADC_CHAN, NUM_ADC_CHAN, adc_data);

    // printf("FT data:: Tz %f\t Ty: %f\t Tx: %f Fz %f\t Fy: %f\t Fx: %f\n", adc_data[0], adc_data[1], adc_data[2], adc_data[3], adc_data[4], adc_data[5]); 
    
    // ///////////////////////////////////////////////////////////////////////////
	// // Initialize ATI FT_data sensor:
	// ///////////////////////////////////////////////////////////////////////////
 
	// const int N_FT_OUT		= 6;  
	// const int FT_INPUT_ON	= 1;  

	// float adc_data_fl[N_FT_OUT + 1] = {0, 0, 0, 0, 0, 0, 0}; 
	// float FT_data[N_FT_OUT]         = {0, 0, 0, 0, 0, 0}; 

    // cout << "Initial F/T sensor !!!" << endl;  
	// init_ft_sensor_ati(CAL_FILE, ADC_CHAN, NUM_ADC_CHAN); 

    // // #define bool FT_INPUT_ON TRUE; 
    // cout << "F/T Sensor Reading Test !!!" << endl; 

    // for(int i = 0; i < 100000; i = i +1)
    // {
    //     printf("Index ::%d\n", i); 

    //     sleep(1); 

    //     if (FT_INPUT_ON) {
                
    //     // Read ADC:
    //     s526_adc_read(ADC_CHAN, NUM_ADC_CHAN, adc_data);

    //     printf("FT data:: Tz %f\t Ty: %f\t Tx: %f Fz %f\t Fy: %f\t Fx: %f\n", adc_data[0], adc_data[1], adc_data[2], adc_data[3], adc_data[4], adc_data[5]); 

    //     // // Convert into forces and torques:
    //     // for (int ft_i = 0; ft_i < NUM_ADC_CHAN; ft_i++)
    //     //     adc_data_fl[ft_i] = (float) adc_data[ft_i]; 

    //     // printf("FT Original data:: Tz %f\t Ty: %f\t Tx: %f Fz %f\t Fy: %f\t Fx: %f\n", adc_data_fl[0], adc_data_fl[1], adc_data_fl[2], adc_data_fl[3], adc_data_fl[4], adc_data_fl[5]); 

    //     // convert_adc_to_ft(adc_data_fl, FT_data); 

    //     // printf("The converted data from FT sensor ::::", FT_data); 
        
    //     // printf("The converted FT data:: Tz %f\t Ty: %f\t Tx: %f Fz %f\t Fy: %f\t Fx: %f\n", FT_data[0], FT_data[1], FT_data[2], FT_data[3], FT_data[4], FT_data[5]);
        
    //     // cout << "Sensor Signals Reading !!!" << endl;  
    // }
    // } 
     
    ///////////////////////////////////////////////////////////////////////////
	// Initialize Encoder and Read: 
	///////////////////////////////////////////////////////////////////////////

    // controller_renishaw encoder("can2"); 

    // cout << "Initial Encoder" << endl; 

   	// // float encoder_arr[2]; 

	// // encoder.read_ang_encoder(encoder_arr); 
    // // cout << "Read analog data" << endl; 

  	// // double theta_sea_in_o = (double) encoder_arr[1] * 3.14/180.0 ; 
  	// // double theta_sea_out_o = (double) encoder_arr[0] * 3.14/180.0 ; 

	// // printf("theta_sea_in_o = [%3.4lf]  theta_sea_out_o = [%3.4lf] \n", theta_sea_in_o,  theta_sea_out_o);

    // // cout << "End Encoder" << endl; 

    // uint8_t received_position[4],received_position2[4];
  	// float position1,position2;

	// received_position[0] = 1;
	// received_position[1] = 84;  
	// received_position[2] = 209;
	// received_position[3] = 67;

    // printf("read data 0:::%d \n", received_position[0]); 
    // printf("read data 1:::%d \n", received_position[1]); 
    // printf("read data 2:::%d \n", received_position[2]); 
    // printf("read data 3:::%d \n", received_position[3]); 

	// encoder.bytes2Float(received_position,&position1); 


    // return 0;  
}
