#include"misc_functions.hpp"

/*bit masking function to seperate upper 6 (node_id) and lower 5 (cmd_id) bits of the can_id in the can frame*/
/*void bit_masking(can_frame_odrive &can_frame)
{
	
	__u32 cmd_id_mask = 0x1F;
	__u32 node_id_mask = 0x7E0;
	can_frame.cmd_id = (cmd_id_mask & can_frame.cframe.can_id) ;
	can_frame.node_id = (node_id_mask & can_frame.cframe.can_id) >> 5;	
    
}


/*architecture dependent float to byte converter (Endian-ness)
*/
void float2Bytes(float float_variable, uint8_t * bytes_temp){ 
  union {
    float a;
    uint8_t bytes[4];
  } thing;
  thing.a = float_variable;
  memcpy(bytes_temp, thing.bytes, 4);
}

//architecture dependent byte to float converter (Endian-ness)
void bytes2Float(uint8_t * bytes_temp, float* float_variable){ 
  union {
    float a;
    uint8_t bytes[4];
  } thing;
  //swap around for different endlian
  thing.bytes[0] = bytes_temp[0];
  thing.bytes[1] = bytes_temp[1];
  thing.bytes[2] = bytes_temp[2];
  thing.bytes[3] = bytes_temp[3];
  *(float_variable) = thing.a;
  //printf("b2f float = %f, bytes_temp: %x %x %x %x \n",thing.a, bytes_temp[0],bytes_temp[1],bytes_temp[2],bytes_temp[3]);
  //printf("b2f *float = %f, thing.bytes: %x %x %x %x \n",*(float_variable), thing.bytes[0],thing.bytes[1],thing.bytes[2],thing.bytes[3]);
		
}


/* Function to reverse arr[] from start to end
void rvereseArray(auto arr[], int start, int end) 
{ 
    while (start < end) 
    { 
        auto temp = arr[start];  
        arr[start] = arr[end]; 
        arr[end] = temp; 
        start++; 
        end--; 
    }  
}      
  
*/