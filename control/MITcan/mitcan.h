#define _MIT_CAN_H_
#define _MIT_CAN_H_

 #define P_MIN -95.5f
 #define P_MAX 95.5f
 #define V_MIN -30.0f
 #define V_MAX 30.0f
 #define KP_MIN 0.0f
 #define KP_MAX 500.0f
 #define KD_MIN 0.0f
 #define KD_MAX 5.0f
 #define T_MIN -18.0f
 #define T_MAX 18.0f

#include "SocketCAN.hpp" 

class Mitcan {
public:
    // Constructor
    Mitcan(CANDevice can_chn): channel_name(can_chn) {} 
    int begin(); 
    struct can_frame readcan(float *pos, float *vel, float *cur); 
    int enable_motor(); 
    int disable_motor(); 
    int writecan(int id,float p_des, float v_des, float kp, float kd, float t_ff); 
    int Set_Velocity(int node,float v_des, float kp, float kd, float t_ff); 
    int Set_Command(int node,float p_des,float v_des, float kp, float kd, float t_ff); 
    int Set_Position(int node,float p_des); 
    void unpack_reply(struct can_frame *msg, float *p, float *v, float *i); 
    void pack_cmd(struct can_frame * msg, int nodeID, float p_des, float v_des, float kp, float kd, float t_ff); 
    void unpack_cmd(struct can_frame* msg);
    int float_to_uint(float x, float x_min, float x_max, int bits);
    float uint_to_float(int x_int, float x_min, float x_max, int bits);

private:
    // Some private functions
    CANDevice channel_name; 
    struct can_frame wframe,rframe; 
    float pos,vel,cur;
};
