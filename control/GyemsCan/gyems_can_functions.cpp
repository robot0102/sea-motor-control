#include "gyems_can_functions.h"

/*****************************
 * Library for control of GYEMS motors 
 * V1.0 by Chang Hong - Dec 2020
 * ******************************/

int Gcan::begin()
{
    channel_name.begin();
    return 0;
}

int Gcan::readcan()
{   
    int i=0;
    while (!channel_name.available()){
        if (i++>10000000){
            printf( "No CAN messages.\n");
            return -1;
        }
    }
    //printf( "CAN messages are available to read.\n");
    channel_name.read(rframe);

    //Note: IF satements are used to filter out the request from the reply. 
    switch(rframe.data[0]){
        case  0x9A:
            status_1_reply(&rframe,&temp,&voltage,&error_state);
            break;
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:

        case 0x9C:
            if(rframe.data[1]!=0)
                unpack_speed_torque_reply(&rframe,&temp,&torque,&speed,&position);
            break;

        case 0xA5:
        case 0xA6:
            if(!(rframe.data[1]==1 || rframe.data[1]==0))
                unpack_speed_torque_reply(&rframe,&temp,&torque,&speed,&position);
            break;

        case 0x9D:
            if(!(rframe.data[6]==0 && rframe.data[7]==0))
                status_3_reply(&rframe,&phase_ai,&phase_bi,&phase_ci);
                break;
        case 0x94:
            // if(!(rframe.data[6]==0 && rframe.data[7]==0))
            unpack_single_turn_angle(&rframe,&position); 
            break;
        case 0x92:
            // if(!(rframe.data[1]==0 && rframe.data[2]==0))
            unpack_multi_turn_angle(&rframe,&multi_turn_position); 
            break;
        case 0x90:
            // if(!(rframe.data[2]==0 && rframe.data[3]==0))
            unpack_read_encoder(&rframe,&position,&raw_position,&encoderoffset);
            break;
        case 0x30:
        case 0x33:
            //do nothing
            break;
        default:
            printf(" Motor %x: unknown ID: %x\n",rframe.can_id,rframe.data[0]);

    }
    return 0;
}

void Gcan::pack_torque_cmd(int nodeID, int16_t iqControl){
     //range :-2000~2000, corresponding to the actual torque current range -32A~32A
     /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0xA1;                                              
     msg.data[1] = 0;                
     msg.data[2] = 0;                   
     msg.data[3] = 0; 
     msg.data[4] = iqControl&0xff;                
     msg.data[5] = (iqControl>>8)&0xff;                 
     msg.data[6] = 0;  
     msg.data[7] = 0;                
    
    //printf("writing: %x %x %x %x (%x %x) %x %x ", msg.data[0],msg.data[1],msg.data[2],msg.data[3],msg.data[4],msg.data[5],msg.data[6],msg.data[7]);
    channel_name.write(msg);
    }

void Gcan::pack_speed_cmd(int nodeID, int32_t speed){
     
    //actual speed of 0.01dps/LSB
    //36000 represents 360°

     /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0xA2;                                 
     msg.data[1] = 0;                 
     msg.data[2] = 0;                   
     msg.data[3] = 0;  
     msg.data[4] = speed&0xff;                
     msg.data[5] = (speed>>8)&0xff;                  
     msg.data[6] = (speed>>16)&0xff;  
     msg.data[7] = (speed>>24)&0xff;                  
    
    //printf("Sent Error %x %x %x %x %x %x %x %x\n",msg.data[0],msg.data[1],msg->data[2],msg->data[3],msg->data[4],msg->data[5],msg->data[6],msg->data[7]);    
    channel_name.write(msg);
    }

void Gcan::unpack_speed_torque_reply(struct can_frame* msg, uint8_t* temp, int16_t* ptorque,int16_t* pspeed,uint16_t* pposition){
//1. Motor temperature（ 1℃/LSB）
//2. Motor torque current(Iq)（Range:-2048~2048,real torque current range:-33A~33A）
//3. Motor speed（1dps/LSB）
//4. Encoder position value（14bit encoder value range 0~16383）
    *temp = msg->data[1];

    *ptorque = (msg->data[3]<<8)|msg->data[2];
    *pspeed = (msg->data[5]<<8)|(msg->data[4]);
    *pposition = (msg->data[7]<<8)|msg->data[6];       
    #ifdef DEBUG
    printf("Temp: %hu toruqe: %d speed: %d pos: %u \n", *temp, *ptorque, *pspeed, *pposition);
    #endif    
}
 

void Gcan::pack_off_cmd(int nodeID){
     
     /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x80;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;               
     msg.data[6] = 0;  
     msg.data[7] = 0;                 
    
    channel_name.write(msg);
    }

void Gcan::pack_stop_cmd(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x81;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;               
     msg.data[6] = 0;  
     msg.data[7] = 0;    
     
    channel_name.write(msg);
    }

void Gcan::pack_run_cmd(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x88;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;               
     msg.data[6] = 0;  
     msg.data[7] = 0;    
     
    channel_name.write(msg);
}
void Gcan::clear_motor_error(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x9B;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;               
     msg.data[6] = 0;  
     msg.data[7] = 0;    
     
    channel_name.write(msg);
}

void Gcan::read_status_1_error(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x9A;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;               
     msg.data[6] = 0;  
     msg.data[7] = 0;    
     
    channel_name.write(msg);
}

void Gcan::status_1_reply(struct can_frame* msg, uint8_t* temp, int16_t* pvoltage,uint8_t* perror_state){
    //NOTE: error flag cannot be cleared when the motor status does not return to normal
    *temp = msg->data[1];
    *pvoltage = (msg->data[4]<<8)|msg->data[3];
    *perror_state = (msg->data[7]);

    #ifdef DEBUG 
    printf("Voltage: %d\n",*pvoltage);
    #endif

    switch (*perror_state) {
        case 1:
            printf("Error: Low voltage protection\n");
            break;
        case 8:
            printf("Overtemperature protection\n");
            break;
        case 0:
            break;
        default:
            printf("Unknown error\n");
            break;
    }   
}

void Gcan::read_status_2_data(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x9C;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;               
     msg.data[6] = 0;  
     msg.data[7] = 0;    
     
    channel_name.write(msg);
}

void Gcan::read_status_3_phase_current(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x9D;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;               
     msg.data[6] = 0;  
     msg.data[7] = 0;    
     
    channel_name.write(msg);
}

void Gcan::status_3_reply(struct can_frame* msg, int16_t* pca,int16_t* pcb,int16_t* pcc){

    *pca = (msg->data[3]<<8)|msg->data[2];
    *pcb = (msg->data[5]<<8)|(msg->data[4]);
    *pcc = (msg->data[7]<<8)|msg->data[6];
    
    #ifdef DEBUG   
    printf("Error: phase current a: %d phase current b: %d phase current c: %d \n", *pca, *pcb, *pcc);
     #endif
}

void Gcan::pack_multi_torque_cmd(int nodeID, int16_t iqControl, int16_t iqControl2, int16_t iqControl3,int16_t iqControl4){
     
     /// pack ints into the can buffer ///
     msg.can_id  = 0x280+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = iqControl&0xff;                                              
     msg.data[1] = (iqControl>>8)&0xff;                
     msg.data[2] = iqControl2&0xff;                                              
     msg.data[3] = (iqControl2>>8)&0xff;                
     msg.data[4] = iqControl3&0xff;                   
     msg.data[5] = (iqControl3>>8)&0xff;                 
     msg.data[6] = iqControl4&0xff;                                              
     msg.data[7] = (iqControl4>>8)&0xff;                
    
    channel_name.write(msg);
    }

void Gcan::pack_position_1_cmd(int nodeID, int32_t angle){
     //multi-turn
    //actual position is 0.01degree/LSB, 36000 represents 360°
     #ifdef DEBUG   
    printf("Error: angle: %ld \n", angle);
     #endif
     
     // keep the outer degree consistent with the inner degree input
     angle = angle * 6;

     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0xA3;                                 
     msg.data[1] = 0;                 
     msg.data[2] = 0;                   
     msg.data[3] = 0;  
     msg.data[4] = angle&0xff;                
     msg.data[5] = (angle>>8)&0xff;                  
     msg.data[6] = (angle>>16)&0xff;  
     msg.data[7] = (angle>>24)&0xff;                  
     
    channel_name.write(msg);
}

void Gcan::pack_position_2_cmd(int nodeID, int32_t angle, uint16_t max_speed){
     //multi-turn
    //actual position is 0.01degree/LSB, 36000 represents 360°
    //actual speed of 1dps/LSB
    #ifdef DEBUG   
    printf("Error: angle: %ld speed: %u \n", angle, max_speed);
     #endif

     // keep the outer degree consistent with the inner degree input
     angle = angle * 6;

     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0xA4;                                 
     msg.data[1] = 0;                 
     msg.data[2] = max_speed&0xff;                   
     msg.data[3] = (max_speed>>8)&0xff;  
     msg.data[4] = angle&0xff;                
     msg.data[5] = (angle>>8)&0xff;                  
     msg.data[6] = (angle>>16)&0xff;  
     msg.data[7] = (angle>>24)&0xff;                  
    
    channel_name.write(msg);
}

void Gcan::pack_position_3_cmd(int nodeID, uint16_t angle, uint8_t spinDirection){
    //single turn

    //actual position is 0.01degree/LSB, the actual angle range is 0°~359.99 . ie. range 0~35999. 
    //0x00 for clockwise and 0x01 for counterclockwise

    #ifdef DEBUG   
    printf("Error: angle: %u spinDirection: %hu\n", angle, spinDirection);
     #endif

     // keep the outer degree consistent with the inner degree input
     angle = angle * 6;

     /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0xA5;                                 
     msg.data[1] = spinDirection;                 
     msg.data[2] = 0;                   
     msg.data[3] = 0;  
     msg.data[4] = angle&0xff;                
     msg.data[5] = (angle>>8)&0xff;                  
     msg.data[6] = 0;  
     msg.data[7] = 0;                  
     
    channel_name.write(msg);
}

void Gcan::pack_position_4_cmd(int nodeID, uint16_t angle, uint16_t max_speed, uint8_t spinDirection){
     //single turn

    //actual position is 0.01degree/LSB, the actual angle range is 0°~359.99 . ie. range 0~35999. 
    //actual speed of 1dps/LSB.
    //0x00 for clockwise and 0x01 for counterclockwise

    #ifdef DEBUG   
    printf("Error: angle: %u max_speed: %u spinDirection: %hu \n", angle, max_speed,spinDirection);
     #endif

     // keep the outer degree consistent with the inner degree input
     angle = angle * 6;

     /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0xA6;                                 
     msg.data[1] = spinDirection;  
     msg.data[2] = max_speed&0xff;                
     msg.data[3] = (max_speed>>8)&0xff;  
     msg.data[4] = angle&0xff;                
     msg.data[5] =   (angle>>8)&0xff;                
     msg.data[6] = 0;  
     msg.data[7] = 0;                  
     
    channel_name.write(msg);
}


void Gcan::read_single_turn_angle(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x94;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;               
     msg.data[6] = 0;  
     msg.data[7] = 0;    
     
    channel_name.write(msg);
}

// return the inner angle value without reduction ratio
void Gcan::unpack_single_turn_angle(struct can_frame* msg, uint16_t* pangle){
    *pangle = (msg->data[7]<<8)|msg->data[6];
    double inner_actual_single_turn_angle = double(*pangle) / 100.0; 
    #ifdef DEBUG   
    printf("Position angle %f \n", inner_actual_single_turn_angle);
     #endif
        
}

void Gcan::read_multi_turn_angle(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x92;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;               
     msg.data[6] = 0;  
     msg.data[7] = 0;    
     
    channel_name.write(msg);
}

// return the inner angle value with reduction ratio
void Gcan::unpack_multi_turn_angle(struct can_frame* msg, int64_t* mangle){
    
    *mangle = (msg->data[7]<<48)|(msg->data[6]<<40)|(msg->data[5]<<32)|(msg->data[4]<<24)|(msg->data[3]<<16)|(msg->data[2]<<8)|(msg->data[1]);
    // double inner_actual_multi_turn_angle = double(*mangle) / 9216000.0 * 360.0;
    printf("data_0 : %d \n, %d \n, %d \n, %d \n, %d \n, %d \n, %d \n", msg->data[0], msg->data[1], msg->data[2], msg->data[3], msg->data[4], msg->data[5], msg->data[6], msg->data[7]);
    printf("Original data : %d \n", (int)*mangle); 
    // printf("Original data : %d \n", double(*mangle));
    #ifdef DEBUG 
    // printf("Multi turn pos: %f \n", inner_actual_multi_turn_angle);
    #endif
}
 

void Gcan::set_pos2zero(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x19;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;               
     msg.data[6] = 0;  
     msg.data[7] = 0;    
     
    channel_name.write(msg);
}


void Gcan::write_encoder_offset(int nodeID,uint16_t offset){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x91;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;              
     msg.data[5] = 0;                  
     msg.data[6] = offset&0xff;  
     msg.data[7] = (offset>>8)&0xff;      
     
    channel_name.write(msg);
}

void Gcan::read_encoder(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x90;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;                  
     msg.data[6] = 0;  
     msg.data[7] = 0;      
     
    channel_name.write(msg);
}

void Gcan::unpack_read_encoder(struct can_frame* msg, uint16_t* pos, uint16_t* raw, uint16_t* offset){
    
    *pos = (msg->data[3]<<8)|msg->data[2];
    *raw = (msg->data[5]<<8)|(msg->data[4]);
    *offset = (msg->data[7]<<8)|msg->data[6];
    #ifdef DEBUG
    printf("pos: %u raw: %u offset: %u \n", *pos, *raw, *offset);
    #endif    
}
 

void Gcan::write_acceleration2ram(int nodeID,uint16_t accel){
    //set acceleration limit
    
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x34;                                              
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;               
     msg.data[4] = accel&0xff;  
     msg.data[5] = (accel>>8)&0xff;  
     msg.data[6] = (accel>>16)&0xff;            
     msg.data[7] = (accel>>24)&0xff;        
     
    channel_name.write(msg);
}

void Gcan::read_acceleration(int nodeID){
    //read acceleration limit

    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x33;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;                  
     msg.data[6] = 0;  
     msg.data[7] = 0;      
     
    channel_name.write(msg);
    usleep(100);
    channel_name.read(rframe);
    unpack_read_acceleration(&rframe,&accel_data);

}

void Gcan::unpack_read_acceleration(struct can_frame* msg, int32_t* paccel){
    
    *paccel = (msg->data[7]<<24)|(msg->data[6]<<16)|(msg->data[5]<<8)|(msg->data[4]);
    
    #ifdef DEBUG
    printf("accel: %ld \n", *paccel);
    #endif    

}

void Gcan::write_PID(int nodeID,PIDconstant pid){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x31;                                            
     msg.data[1] = 0;                
     msg.data[2] = pid.anglePidKp&0xff;  
     msg.data[3] = pid.anglePidKi&0xff;  
     msg.data[4] = pid.speedPidKp&0xff;  
     msg.data[5] = pid.speedPidKi&0xff;             
     msg.data[6] = pid.iqPidKp&0xff;  
     msg.data[7] = pid.iqPidKi&0xff;  
     
    channel_name.write(msg);
}


void Gcan::read_PID(int nodeID){
     
    /// pack ints into the can buffer ///
     msg.can_id  = 0x140+nodeID;
	 msg.can_dlc = 8;
	 msg.data[0] = 0x30;                                            
     msg.data[1] = 0;                
     msg.data[2] = 0;                  
     msg.data[3] = 0;  
     msg.data[4] = 0;            
     msg.data[5] = 0;                  
     msg.data[6] = 0;  
     msg.data[7] = 0;      
     
    channel_name.write(msg);
    usleep(100);
    channel_name.read(rframe);
    unpack_read_PID(&rframe,&pid);
}

void Gcan::unpack_read_PID(struct can_frame* msg, struct PIDconstant* pid){
    
    pid->anglePidKp = msg->data[2];
    pid->anglePidKi = msg->data[3];
    pid->speedPidKp = msg->data[4];
    pid->speedPidKi = msg->data[5];
    pid->iqPidKp = msg->data[6];
    pid->iqPidKi = msg->data[7];
    
    #ifdef DEBUG
    printf("anglePidKp: %hu anglePidKi: %hu \n", pid->anglePidKp, pid->anglePidKi);
    printf("speedPidKp: %hu speedPidKi: %hu \n", pid->speedPidKp, pid->speedPidKi);
    printf("iqPidKp: %hu iqPidKi: %hu \n", pid->iqPidKp, pid->iqPidKi);
    #endif    
}
 
