#ifndef _MF_H_
#define _MF_H_


#include<cstring>
#include"innfos_can_functions.hpp"

using namespace std;

//void bit_masking(can_frame_odrive &can_frame);
void float2Bytes(float float_variable, uint8_t *bytes_temp);
void bytes2Float(uint8_t *bytes_temp, float *float_variable);
//void rvereseArray(auto arr[], int start, int end);

#endif /* _MF_H_ */
