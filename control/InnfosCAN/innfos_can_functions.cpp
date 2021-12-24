/* Created by Sumantra on 9th December 2019 */
/* Ver 1.1 Major bug fix by Chang Hong 10 Dec 2019 */

#include "innfos_can_functions.hpp"
#include "misc_functions.hpp"

/*CAN READ AND WRITE FUNCTIONS*/

controller::controller(int writesocket_fd,int readsocket_fd)
{
	
	/*initialize rx and tx can messages structs*/
	for (int i = 0; i < 7; ++i)
	{	
		this->tx_msg.cframe.data[i] = 0;
		this->rx_msg.cframe.data[i] = 0;
	}
	this->tx_msg.cframe.can_dlc = 8; 
	this->rx_msg.cframe.can_dlc = 8; 
	write_socket = writesocket_fd;
	read_socket = writesocket_fd;

}

controller::
controller(const char *can_iface_name) // TODO
{
    // Initialize CAN Bus Sockets
    int s; 
    struct sockaddr_can addr1; 
    //struct can_frame frame; 
    struct ifreq ifr;

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Error while opening socket!");
    }

    strcpy(ifr.ifr_name, can_iface_name);
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr1.can_family  = AF_CAN;
    addr1.can_ifindex = ifr.ifr_ifindex;

    if(bind(s, (struct sockaddr *)&addr1, sizeof(addr1)) < 0) {
        perror("Error in socket bind");
    }
    
    // initialize rx and tx can messages structs:
    for (int i = 0; i < 7; ++i)
    {	
        this->tx_msg.cframe.data[i] = 0;
        this->rx_msg.cframe.data[i] = 0;
    }
    this->tx_msg.cframe.can_dlc = 8; 
    this->rx_msg.cframe.can_dlc = 8; 
    write_socket = s;
    read_socket = 0;
    printf("read socket: %d, write socket: %d\n",read_socket,write_socket );
}

bool controller::can_read()
{	
	//struct can_frame sframe;	
  	int nbytes;
	//printf("tread 2 socket: %d can_id: %x\n",read_socket,sframe.can_id);	
	nbytes = read(this->write_socket, &(tx_msg.cframe), sizeof(struct can_frame));
	
    // Paranoid check:
	/*
    if (nbytes < sizeof(struct can_frame)) {
            perror("read: incomplete CAN frame\n");
            return 1;
    }
	*/

	return 0; 
}

bool controller::can_write()
{	
	int nbytes;
	// printf("can_write %d:",write_socket);
	
	nbytes = write(this->write_socket, &(tx_msg.cframe), sizeof(struct can_frame));

	if (nbytes < 0)	{
		printf("\n\ncontroller::can_write(): errno = %d [%s]\n", errno, strerror(errno));
		exit(0);
	}

    // Paranoid check:
	/*
    if (nbytes < sizeof(struct can_frame)) {
		perror("write: incomplete CAN frame\n");
		return 1;
    }
	*/

	return 0;
}

/*MSG HANDLER FUNCTIONS FOR PROCESSING AND ORGANIZING INCOMING MSGS*/
float controller::read_pos_setpoint(int node_id) {

	const int NUM_ATTEMPTS = 10;
	const float ERR_VAL = 0.0;

	uint8_t a,b,c,d;
	int32_t received_position;
	float final_position;
	int i = NUM_ATTEMPTS;

	this->tx_msg.cframe.can_id  = node_id;
	this->tx_msg.cframe.can_dlc = SIZE_READ;

	this->tx_msg.cframe.data[0] = GET_POS;
	//most significant bit at the left hand side

	int nbytes;	

	can_write();

	//tx_msg.cframe.can_dlc = SIZE_READ_RETURN_3;
	tx_msg.cframe.data[0] = 0;

	while (tx_msg.cframe.data[0] != GET_POS){
		can_read();

		i--;
		if (i<0){
			perror("read_pos_setpoint(): return message not detected");
			return ERR_VAL;
		}
	}	

	a = this->tx_msg.cframe.data[1];
	b = this->tx_msg.cframe.data[2];
	c = this->tx_msg.cframe.data[3];
	d = this->tx_msg.cframe.data[4];

	received_position = (d | c << 8 | b << 16 | a << 24);
	// printf("position = %ld, breakdown: %x %x %x %x %x\n",received_position, tx_msg.cframe.data[1], a,b,c,d); 

	final_position = float(received_position)/IQ_24;
	// printf("final_position = %f\n",final_position);

	return final_position;	
}

float controller::read_vel_setpoint(int node_id) {

	uint8_t a,b,c,d;
	int32_t received_velocity;
	float final_velocity;
	int i=10;

	this->tx_msg.cframe.can_id = node_id;
	this->tx_msg.cframe.can_dlc = SIZE_READ;

	this->tx_msg.cframe.data[0] = GET_VEL;
	//most significant bit at the left ahnd side

	int nbytes;	
	can_write();
	//tx_msg.cframe.can_dlc = SIZE_READ_RETURN_3;

	tx_msg.cframe.data[0] = 0;

	while (tx_msg.cframe.data[0] != GET_VEL){
		can_read();

		if (tx_msg.cframe.data[0] == GET_VEL){

			a = this->tx_msg.cframe.data[1];
			b = this->tx_msg.cframe.data[2];
			c = this->tx_msg.cframe.data[3];
			d = this->tx_msg.cframe.data[4];

			received_velocity = (d | c << 8 | b << 16 | a << 24);
			// printf("velocity = %ld, breakdown: %x %x %x %x %x\n",received_velocity, tx_msg.cframe.data[1], a,b,c,d);

			final_velocity = float(received_velocity)/IQ_24*MAX_SPEED;
			// printf("final_velocity = %f\n",final_velocity);

			return final_velocity;
		}
		i--;
		if (i==0){
			perror("return message not detected");
			return -1.0;
		}
	}

	return 0.0;
}

float controller::read_cur_setpoint(int node_id) {

	uint8_t a,b,c,d;
	int32_t received_current;
	float final_current;
	int i=10;
	
	this->tx_msg.cframe.can_id = node_id;
	this->tx_msg.cframe.can_dlc = SIZE_READ;

	this->tx_msg.cframe.data[0] = GET_CUR;
	//most significant bit at the left ahnd side

	int nbytes;	
	can_write();
	//tx_msg.cframe.can_dlc = SIZE_READ_RETURN_3;

	tx_msg.cframe.data[0] = 0;

	while (tx_msg.cframe.data[0] != GET_CUR){
		can_read();
	
		if (tx_msg.cframe.data[0] == GET_CUR){

			a = this->tx_msg.cframe.data[1];
			b = this->tx_msg.cframe.data[2];
			c = this->tx_msg.cframe.data[3];
			d = this->tx_msg.cframe.data[4];  

			received_current = (d | c << 8 | b << 16 | a << 24);
			// printf("current = %ld, breakdown: %x %x %x %x %x\n",received_current, tx_msg.cframe.data[1], a,b,c,d);

			final_current = float(received_current)/IQ_24*MAX_CURRENT;

			// printf("final_current = %f\n",final_current);

			return final_current;
		}
		i--;
		if (i==0){
			perror("return message not detected");
			return -1.0;
		}

	}

	return 0.0;

}

void controller::set_pos_setpoint(int node_id, float pos_setpoint) {
	//pos_setpoint between -128 to 127.999

	int32_t i, j,full_steps,send_position;
	this->tx_msg.cframe.can_id = node_id;
	
	float current_pos,first_turn, turns_required,last_turn, next_turn =0;

	current_pos = read_pos_setpoint(node_id);
	turns_required = pos_setpoint - current_pos;

	if (current_pos<-128.0) //read errror
		return;

	this->tx_msg.cframe.data[0] = SET_POS_SETPOINT;
	this->tx_msg.cframe.can_dlc = SIZE_WRITE_3;
	
	if (turns_required>0){
		full_steps = floor(turns_required);
		last_turn = turns_required - full_steps;
		for (i=0;i<full_steps;i++){
			current_pos += 0.9999;
			send_position=int(round(current_pos*IQ_24));	
				
			this->tx_msg.cframe.data[4] = (send_position & BIT_MASK_0) ;
			this->tx_msg.cframe.data[3] = (send_position & BIT_MASK_1) >> 8;
			this->tx_msg.cframe.data[2] = (send_position & BIT_MASK_2) >> 16;
			this->tx_msg.cframe.data[1] = (send_position & BIT_MASK_3) >> 24;
			can_write();
			printf("position %f R set to %d: (%x)\n",current_pos, send_position,send_position);
			usleep(500000);
		}
	}else{
		full_steps = ceil(turns_required);
		last_turn = turns_required - full_steps;
		for (i=0;i>full_steps;i--){
			current_pos -= 0.9999;
			send_position=int(round(current_pos*IQ_24));	
			
			this->tx_msg.cframe.data[4] = (send_position & BIT_MASK_0) ;
			this->tx_msg.cframe.data[3] = (send_position & BIT_MASK_1) >> 8;
			this->tx_msg.cframe.data[2] = (send_position & BIT_MASK_2) >> 16;
			this->tx_msg.cframe.data[1] = (send_position & BIT_MASK_3) >> 24;
			can_write();
			printf("position %f R set to %d: (%x)\n",current_pos, send_position,send_position);
			usleep(500000);
		}
	}

	//suggestion: explore more efficient turning code by calulating only the changes in just last biggest 4 bits
	//hexadecimal representation of postiion change
	//80 00 00 00 (-128) to FF 00 00 00(-1)   			each turn FE 00 00 00(-2) 
	//00 00 00 00 (0)    to 7F FF FF FF (127.99999994)   each turn 0 FF FF FF (1)
	printf("last step\n");
			
	send_position=int(round(pos_setpoint*IQ_24));
	
	//most significant bit at the left hand side
	this->tx_msg.cframe.data[4] = (send_position & BIT_MASK_0) ;
	this->tx_msg.cframe.data[3] = (send_position & BIT_MASK_1) >> 8;
	this->tx_msg.cframe.data[2] = (send_position & BIT_MASK_2) >> 16;
	this->tx_msg.cframe.data[1] = (send_position & BIT_MASK_3) >> 24;

	can_write();

	printf("setpoint position %f R set to %d: (%x)\n",last_turn, send_position,send_position);
	printf("position %f R reached\n",pos_setpoint, send_position,send_position);
	
}

void controller::set_vel_setpoint(int node_id, float vel_setpoint)
{
	//vel_setpoint between -6000 to 6000

	int32_t send_velocity=int(round((vel_setpoint/MAX_SPEED)*IQ_24));
	
	this->tx_msg.cframe.can_id = node_id;
	this->tx_msg.cframe.can_dlc = SIZE_WRITE_3;

	this->tx_msg.cframe.data[0] = SET_VEL_SETPOINT;
	//most significant bit at the left hand side
	this->tx_msg.cframe.data[4] = (send_velocity & BIT_MASK_0) ;
	this->tx_msg.cframe.data[3] = (send_velocity & BIT_MASK_1) >> 8;
	this->tx_msg.cframe.data[2] = (send_velocity & BIT_MASK_2) >> 16;
	this->tx_msg.cframe.data[1] = (send_velocity & BIT_MASK_3) >> 24;

	can_write();
	// printf("speed %f rpm set to %d: (%x)\n",vel_setpoint, send_velocity,send_velocity);
	
}

void controller::set_cur_setpoint(int node_id, float cur_setpoint)
{
	//cur_setpoint between -33 to 33

	int32_t send_current=int(round((cur_setpoint/MAX_CURRENT)*IQ_24));
	
	this->tx_msg.cframe.can_id = node_id;
	this->tx_msg.cframe.can_dlc = SIZE_WRITE_3;

	this->tx_msg.cframe.data[0] = SET_CUR_SETPOINT;
	//most significant bit at the left hand side
	this->tx_msg.cframe.data[4] = (send_current & BIT_MASK_0) ;
	this->tx_msg.cframe.data[3] = (send_current & BIT_MASK_1) >> 8;
	this->tx_msg.cframe.data[2] = (send_current & BIT_MASK_2) >> 16;
	this->tx_msg.cframe.data[1] = (send_current & BIT_MASK_3) >> 24;

	can_write();
	// printf("current [%f] A set to [%d (%x)]\n", cur_setpoint, send_current, send_current);
}

void controller::enable_motor(int node_id)
{

	this->tx_msg.cframe.can_id = node_id;
	this->tx_msg.cframe.can_dlc = SIZE_WRITE_1;

	this->tx_msg.cframe.data[0] = ENABLE_DISABLE_MOTOR;
	this->tx_msg.cframe.data[1] = 0x1;

	can_write();
	printf("enabling %d motor of nodeid %d\n", node_id, tx_msg.node_id);
}

void controller::disable_motor(int node_id)
{

	this->tx_msg.cframe.can_id = node_id;
	this->tx_msg.cframe.can_dlc = SIZE_WRITE_1;

	this->tx_msg.cframe.data[0] = ENABLE_DISABLE_MOTOR;
	this->tx_msg.cframe.data[1] = 0x0;
	can_write();
	printf("disabling motor\n");
}
	
void controller::change_mode(int node_id, uint8_t mode)
{

	this->tx_msg.cframe.can_id = node_id;
	this->tx_msg.cframe.can_dlc = SIZE_WRITE_1;

	this->tx_msg.cframe.data[0] = CHANGE_MODE;
	this->tx_msg.cframe.data[1] = mode;
	can_write();
	printf("change mode to %d\n",mode);
}
	


	