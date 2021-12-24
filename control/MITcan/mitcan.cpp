#include "mitcan.h"


int Mitcan::begin()
{
    channel_name.begin(); 
    return 0; 
}

int Mitcan::enable_motor()
{
    wframe.can_id = 1; 
    wframe.can_dlc = 8; 
    wframe.data[0] = 255; 
    wframe.data[1] = 255; 
    wframe.data[2] = 255; 
    wframe.data[3] = 255; 
    wframe.data[4] = 255; 
    wframe.data[5] = 255; 
    wframe.data[6] = 255; 
    wframe.data[7] = 252; 
    channel_name.write(wframe); 
    printf("<Message::: %x \n", wframe.data[5]); 
    printf("<Message::: %x \n", wframe.data[6]); 
    printf("<Message::: %x \n", wframe.data[7]); 
    return 0; 
}

int Mitcan::disable_motor()
{
    wframe.can_id = 1; 
    wframe.can_dlc = 8; 
    wframe.data[0] = 255; 
    wframe.data[1] = 255; 
    wframe.data[2] = 255; 
    wframe.data[3] = 255; 
    wframe.data[4] = 255; 
    wframe.data[5] = 255; 
    wframe.data[6] = 255; 
    wframe.data[7] = 253; 
    channel_name.write(wframe); 
    return 0; 
}

struct can_frame Mitcan::readcan(float *re_pos, float *re_vel, float *re_cur)
{
    int i=0;
    while (!channel_name.available()){
        if (i++>100000000){
            printf( "No CAN messages.\n");
        }
    }

    //printf( "CAN messages are available to read.\n"); 
    channel_name.read(rframe); 
    unpack_cmd(&rframe); 
    unpack_reply(&rframe, &pos, &vel, &cur); 

    *re_pos = pos; 
    *re_vel = vel; 
    *re_cur = cur;
    return rframe; 

    /*int ret = channel_name.available(); 
    if (ret > 0) {
        printf( "CAN messages are available to read.\n"); 
        channel_name.read(rframe); 
        //unpack_reply(&rframe,&pos,&vel,&cur); 
        unpack_cmd(&rframe);
        return 0;
    }
    else {
       printf( "No CAN messages.\n");
        return 1; 
    }*/
}

int Mitcan::writecan(int id, float p_des, float v_des, float kp, float kd, float t_ff) 
{
    pack_cmd(&wframe, id, p_des, v_des, kp, kd, t_ff); 
    channel_name.write(wframe); 
    return 0; 
}

int Mitcan::Set_Velocity(int node,float v_des, float kp, float kd, float t_ff)
{
    writecan(node, 0, v_des, kp, kd, t_ff); 
    return 0; 
}

int Mitcan::Set_Command(int node,float p_des,float v_des, float kp, float kd, float t_ff)
{
    writecan(node, p_des, v_des, kp, kd, t_ff); 
    return 0; 
}

int Mitcan::Set_Position(int node,float p_des)
{
    writecan(node, p_des, 0, 1, 0, 0); 
    return 0; 
}


/// CAN Reply Packet Structure //
/// 16 bit position, between -4*pi and 4*pi
/// 12 bit velocity, between -30 and + 30 rad/s
/// 12 bit current, between -40 and 40;
/// CAN Packet is 5 8-bit words
/// Formatted as follows.  For each quantity, bit 0 is LSB
/// 0: [position[15-8]]
/// 1: [position[7-0]] 
/// 2: [velocity[11-4]]
/// 3: [velocity[3-0], current[11-8]]
/// 4: [current[7-0]]
void Mitcan::unpack_reply(struct can_frame *msg, float *p, float *v, float *i){
    /// unpack ints from can buffer ///
    if (msg->can_id ==0){
        int16_t p_int=0, v_int=0, i_int=0, v_int2=0;
        int id = msg->data[0];                              //驱动ID号
        p_int = (msg->data[1]<<8)|msg->data[2];             //电机位置数据
        v_int = (msg->data[3]<<4)|((msg->data[4]>>4)&0xF);         //电机速度数据
        i_int = ((msg->data[4]&0xF)<<8)|msg->data[5];        //电机扭矩数据
        
        //printf("postion: %d/t speed: %d\t torque: %d\n", p_int,v_int,i_int); 
        
        /// convert ints to floats ///
        *p = uint_to_float(p_int, P_MIN, P_MAX, 16); 
        *v = uint_to_float(v_int, V_MIN, V_MAX, 12); 
        *i = uint_to_float(i_int, -T_MAX, T_MAX, 12); 
        
        printf("motor %d: postion: %f\t speed: %f\t torque: %f\n", id, *p,*v,*i); 
        
    }
}

/// CAN Command Packet Structure ///
/// 16 bit position command, between -4*pi and 4*pi
/// 12 bit velocity command, between -30 and + 30 rad/s
/// 12 bit kp, between 0 and 500 N-m/rad
/// 12 bit kd, between 0 and 100 N-m*s/rad
/// 12 bit feed forward torque, between -18 and 18 N-m
/// CAN Packet is 8 8-bit words
/// Formatted as follows.  For each quantity, bit 0 is LSB
/// 0: [position[15-8]]
/// 1: [position[7-0]] 
/// 2: [velocity[11-4]]
/// 3: [velocity[3-0], kp[11-8]]
/// 4: [kp[7-0]]
/// 5: [kd[11-4]]
/// 6: [kd[3-0], torque[11-8]]
/// 7: [torque[7-0]]
 void Mitcan::pack_cmd(struct can_frame * msg, int nodeID, float p_des, float v_des, float kp, float kd, float t_ff){
     /// limit data to be within bounds ///
     p_des = fminf(fmaxf(P_MIN, p_des), P_MAX);                    
     v_des = fminf(fmaxf(V_MIN, v_des), V_MAX);
     kp = fminf(fmaxf(KP_MIN, kp), KP_MAX);
     kd = fminf(fmaxf(KD_MIN, kd), KD_MAX);
     t_ff = fminf(fmaxf(T_MIN, t_ff), T_MAX); 
     //printf("Sent %f %f %f %f %f\n",p_des,v_des,kp,kd,t_ff);
    
     /// convert floats to unsigned ints ///
     int p_int = float_to_uint(p_des, P_MIN, P_MAX, 16);            
     int v_int = float_to_uint(v_des, V_MIN, V_MAX, 12); 
     int kp_int = float_to_uint(kp, KP_MIN, KP_MAX, 12); 
     int kd_int = float_to_uint(kd, KD_MIN, KD_MAX, 12); 
     int t_int = float_to_uint(t_ff, T_MIN, T_MAX, 12); 
     //printf("Sent %d %d %d %d %d\n",p_int,v_int,kp_int,kd_int,t_int);
    
     /// pack ints into the can buffer ///
     msg->can_id  = nodeID; 
	 msg->can_dlc = 8;
	 msg->data[0] = p_int>>8;                   //位置高8                            
     msg->data[1] = p_int&0xFF;                 //位置低8     
     msg->data[2] = v_int>>4;                   //速度高8位
     msg->data[3] = ((v_int&0xF)<<4)|(kp_int>>8);  //速度低4位   KP高4位
     msg->data[4] = kp_int&0xFF;                //KP低8位
     msg->data[5] = kd_int>>4;                  //Kd高8位
     msg->data[6] = ((kd_int&0xF)<<4)|(t_int>>8);  //KP低4位     扭矩高4位
     msg->data[7] = t_int&0xff;                  //扭矩低8位  
    
    //printf("Sent Error %x %x %x %x %x %x %x %x\n",msg->data[0],msg->data[1],msg->data[2],msg->data[3],msg->data[4],msg->data[5],msg->data[6],msg->data[7]);
    //printf("Received   ");
    //printf("%.3f  %.3f  %.3f  %.3f  %.3f   %.3f", controller->p_des, controller->v_des, controller->kp, controller->kd, controller->t_ff, controller->i_q_ref);
    //printf("\n\r"); 
     
    }

void Mitcan::unpack_cmd(struct can_frame* msg){ 
        if (msg->can_id !=0){  
            int p_int = (msg->data[0]<<8)|msg->data[1];  
            int v_int = (msg->data[2]<<4)|(msg->data[3]>>4);  
            int kp_int = ((msg->data[3]&0xF)<<8)|msg->data[4];  
            int kd_int = (msg->data[5]<<4)|(msg->data[6]>>4);  
            int t_int = ((msg->data[6]&0xF)<<8)|msg->data[7];  
            
            //controller->p_des = uint_to_float(p_int, P_MIN, P_MAX, 16); 
            //controller->v_des = uint_to_float(v_int, V_MIN, V_MAX, 12); 
            //controller->kp = uint_to_float(kp_int, KP_MIN, KP_MAX, 12); 
            //controller->kd = uint_to_float(kd_int, KD_MIN, KD_MAX, 12);
            //controller->t_ff = uint_to_float(t_int, T_MIN, T_MAX, 12);
            //printf("Received %d %d %d %d %d\n",p_int,v_int,kp_int,kd_int,t_int);

            float p_des = uint_to_float(p_int, P_MIN, P_MAX, 16);  
            float v_des = uint_to_float(v_int, V_MIN, V_MAX, 12);  
            float kp = uint_to_float(kp_int, KP_MIN, KP_MAX, 12);  
            float kd = uint_to_float(kd_int, KD_MIN, KD_MAX, 12);  
            float t_ff = uint_to_float(t_int, T_MIN, T_MAX, 12);  
            printf("Error: %.3f  %.3f  %.3f  %.3f  %.3f\n", p_des, v_des, kp, kd, t_ff); 
        }
    //printf("Received   "); 
    //printf("\n\r");
    }
 


int Mitcan::float_to_uint(float x, float x_min, float x_max, int bits){
    /// Converts a float to an unsigned int, given range and number of bits ///
    float span = x_max - x_min; 
    float offset = x_min; 
    // if (x==0){ 
    //     return 0; 
    // }
    return (int) ((x-offset)*((float)((1<<bits)-1))/span); 
    }
    
    
float Mitcan::uint_to_float(int x_int, float x_min, float x_max, int bits){
    /// converts unsigned int to float, given range and number of bits ///
    float span = x_max - x_min; 
    float offset = x_min; 
    // if (x_int==0){
    //     return 0; 
    // }
    return ((float)x_int)*span/((float)((1<<bits)-1)) + offset; 
    }

