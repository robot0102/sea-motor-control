#ifndef _GYEMS_CAN_H_
#define _GYEMS_CAN_H_

#define P_MIN -12.5f
#define P_MAX 12.5f
#define V_MIN -45.0f
#define V_MAX 45.0f
#define KP_MIN 0.0f
#define KP_MAX 500.0f
#define KD_MIN 0.0f
#define KD_MAX 5.0f
#define T_MIN -18.0f
#define T_MAX 18.0f

#include "SocketCAN.hpp"
#include <inttypes.h>

#define DEBUG

struct PIDconstant{
    uint8_t anglePidKp;
    uint8_t anglePidKi;
    uint8_t speedPidKp;
    uint8_t speedPidKi;
    uint8_t iqPidKp;
    uint8_t iqPidKi;
};

class Gcan {
public:
    // Constructor
    Gcan(CANDevice can_chn): channel_name(can_chn) {}
    int begin();
    int readcan();
    void pack_torque_cmd(int nodeID, int16_t iqControl);
    void pack_speed_cmd(int nodeID, int32_t speed);
    void unpack_speed_torque_reply(struct can_frame* msg, uint8_t* temp, int16_t* ptorque,int16_t* pspeed,uint16_t* pposition);
    void pack_off_cmd(int nodeID);
    void pack_stop_cmd(int nodeID);
    void pack_run_cmd(int nodeID);
    void clear_motor_error(int nodeID);
    void read_status_1_error(int nodeID);
    void status_1_reply(struct can_frame* msg, uint8_t* temp, int16_t* pvoltage,uint8_t* perror_state);
    void read_status_2_data(int nodeID);

    void read_status_3_phase_current(int nodeID);
    void status_3_reply(struct can_frame* msg, int16_t* pca,int16_t* pcb,int16_t* pcc);
    
    void pack_multi_torque_cmd(int nodeID, int16_t iqControl, int16_t iqControl2, int16_t iqControl3,int16_t iqControl4);
    void pack_position_1_cmd(int nodeID, int32_t angle);
    void pack_position_2_cmd(int nodeID, int32_t angle, uint16_t max_speed);
    void pack_position_3_cmd(int nodeID, uint16_t angle, uint8_t spinDirection);
    void pack_position_4_cmd(int nodeID, uint16_t angle, uint16_t max_speed, uint8_t spinDirection);
    void read_single_turn_angle(int nodeID);
    void unpack_single_turn_angle(struct can_frame* msg, uint16_t* pangle);
    void read_multi_turn_angle(int nodeID);
    void unpack_multi_turn_angle(struct can_frame* msg, int64_t* mangle);
    void set_pos2zero(int nodeID);      
    void write_encoder_offset(int nodeID,uint16_t offset);          
    void read_encoder(int nodeID);      
    void unpack_read_encoder(struct can_frame* msg, uint16_t* pos, uint16_t* raw, uint16_t* offset);
    
    void write_acceleration2ram(int nodeID,uint16_t accel);
    void read_acceleration(int nodeID);
    void unpack_read_acceleration(struct can_frame* msg, int32_t* paccel);
    void write_PID(int nodeID,PIDconstant pid);
    void read_PID(int nodeID);
    void unpack_read_PID(struct can_frame* msg, PIDconstant* pid);
 
private:
    // Some private functions
    CANDevice channel_name;
    struct can_frame msg,rframe;

    uint8_t temp,error_state;
    int16_t voltage,torque,speed,phase_ai, phase_bi,phase_ci;
    uint16_t position,raw_position,encoderoffset;
    int32_t accel_data;
    int64_t multi_turn_position;
    PIDconstant pid;
};

 
 


#endif