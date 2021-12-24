#include <iostream>
#include <string> 

using namespace std; 

#include "mitcan.h" 

// int float_to_uint(float x, float x_min, float x_max, int bits){
//     /// Converts a float to an unsigned int, given range and number of bits ///
//     float span = x_max - x_min; 
//     float offset = x_min; 
//     return (int) ((x-offset)*((float)((1<<bits)-1))/span); 
// }

// int test_value; 
int main(){
    int test_p_des, test_v_des, test_kp, test_kd, test_torque; 
    
    cout << "initial can" << endl; 
    CANDevice can0((char *) "can0"); 
    CANDevice can1((char *) "can1"); 
    CANDevice can2((char *) "can2"); 
    CANDevice can3((char *) "can3"); 

    // CANDevice can1((char *) "can1"); 
    can0.begin(); 
    can1.begin(); 
    can2.begin(); 
    can3.begin(); 

    Mitcan can_test(can0); 
    Mitcan can_test_1(can1); 
    Mitcan can_test_2(can2); 
    Mitcan can_test_3(can3); 

    cout << "Enable motor !!!" << endl;  

    // Mitcan can1; 
    can_test.enable_motor(); 
    cout << "CAN 0" << endl; 

    can_test_1.enable_motor(); 
    cout << "CAN 1" << endl; 

    // cout << "Disable motor !!!" << endl; 

    // can_test_1.Set_Command(1, 0.5, 0., 5.0, 0.4, 0.); 
    // cout << "Send position !!!" << endl; 

    float pos_1, vel_1, cur_1; 
    struct can_frame rframe_1 = can_test.readcan(&pos_1, &vel_1, &cur_1); 
    cout << "pos_1" << pos_1 << endl; 
    cout << "vel_1" << vel_1 << endl; 
    cout << "cur_1" << cur_1 << endl; 

    // cout << "Read CAN !!!" << endl; 
    float pos_2, vel_2, cur_2; 
    struct can_frame rframe_2 = can_test_1.readcan(&pos_2, &vel_2, &cur_2); 
    cout << "pos_2" << pos_2 << endl; 
    cout << "vel_2" << vel_2 << endl; 
    cout << "cur_2" << cur_2 << endl; 

    // can_test.disable_motor(); 
    // can_test_1.disable_motor(); 
    // cout << "Disable motor !!!" << endl; 
    
    // return 0;  
}
