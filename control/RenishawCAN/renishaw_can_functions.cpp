/* Created by Sumantra on 9th December 2019 */
/* Ver 1.1 Major bug fix by Chang Hong 10 Dec 2019 */
/* Ver 1.2 modified by Yuepeng for renishaw encoders 18 July 2020 */

#include "renishaw_can_functions.hpp"

/*CAN READ AND WRITE FUNCTIONS*/

controller_renishaw::
controller_renishaw(int writesocket_fd,int readsocket_fd)
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

controller_renishaw::
controller_renishaw(const char *can_iface_name) // TODO
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

int controller_renishaw::available()
{
    fd_set rfds;
    struct timespec tv;
    int retval;

    // Zero the set of readable file descriptors.
    FD_ZERO(&rfds);
    // Add socket to the set of File descriptors to check for read readiness.
    FD_SET(s, &rfds);

    /* Don't wait, just check */
    tv.tv_sec = 0; 
    tv.tv_nsec = timeout_us*1000; 

    // Check of read readiness.
    retval = pselect(FD_SETSIZE, &rfds, NULL, NULL, &tv, NULL);

    if (retval == -1) {
        perror("Error during pselect()");
        return -1;
    }
    else if (retval) {
        // Data is available now. FD_ISSET(s, &rfds) will be true
        return 1;
    }
    else {
        // No Data is available
        return 0;
    }
}


int controller_renishaw::can_read()
{	
	//struct can_frame sframe;	
  	int nbytes;
	//printf("tread 2 socket: %d can_id: %x\n",read_socket,sframe.can_id);	
    
	nbytes = read(this->write_socket, &(tx_msg.cframe), sizeof(struct can_frame)); 
	
    cout << "read can id" << tx_msg.cframe.can_id << endl; 

    // Paranoid check:
	/*
    if (nbytes < sizeof(struct can_frame)) { 
            perror("read: incomplete CAN frame\n"); 
    }
	*/

	return 0; 
}

int controller_renishaw::can_write()
{	
	int nbytes;
	// printf("can_write %d:",write_socket);
	
	nbytes = write(this->write_socket, &(tx_msg.cframe), sizeof(struct can_frame));

    // cout << tx_msg.cframe.data[0] << endl;
    cout << "write can id \n" << tx_msg.cframe.can_id << endl; 

	if (nbytes < 0)	{
		printf("\n\ncontroller_renishaw::can_write(): errno = %d [%s]\n", errno, strerror(errno));
		// exit(0); 
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


void controller_renishaw::bytes2Float(uint8_t * bytes_temp, float* float_variable){ 
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
  printf("b2f float = %f, bytes_temp: %x %x %x %x \n",thing.a, bytes_temp[0],bytes_temp[1],bytes_temp[2],bytes_temp[3]);
  printf("b2f *float = %f, thing.bytes: %x %x %x %x \n",*(float_variable), thing.bytes[0],thing.bytes[1],thing.bytes[2],thing.bytes[3]);
		
}

void controller_renishaw::read_ang_encoder(float (&data_arr)[2]) {

    this->tx_msg.cframe.can_id  = 0x400;
    can_write();

    cout << "can write !!!" << endl;

    this->tx_msg.cframe.can_id  = 0x401;
	can_read();

    cout << "can read !!!" << endl; 

	uint8_t received_position[4],received_position2[4];
  	float position1,position2;

	received_position[0] = this->tx_msg.cframe.data[0];
	received_position[1] = this->tx_msg.cframe.data[1];
	received_position[2] = this->tx_msg.cframe.data[2];
	received_position[3] = this->tx_msg.cframe.data[3];

	received_position2[0] = this->tx_msg.cframe.data[4];
	received_position2[1] = this->tx_msg.cframe.data[5];
	received_position2[2] = this->tx_msg.cframe.data[6];
	received_position2[3] = this->tx_msg.cframe.data[7]; 

    printf("read data 0:::%d \n", received_position[0]); 
    printf("read data 1:::%d \n", received_position[1]); 
    printf("read data 2:::%d \n", received_position[2]); 
    printf("read data 3:::%d \n", received_position[3]); 

	bytes2Float(received_position,&position1); 
	bytes2Float(received_position2,&position2);
 
  	data_arr[0]=position1;
  	data_arr[1]=position2;	
	
	// printf("received_position = [%x]  received_position2 = [%x] \n", received_position[0],  received_position[1]);
	// printf("data_arr[0]= [%3.4lf]  data_arr[1] = [%3.4lf] \n", data_arr[0],  data_arr[1]);
}





	