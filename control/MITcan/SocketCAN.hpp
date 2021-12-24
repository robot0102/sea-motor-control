#ifndef _SOCKET_CAN_H_
#define _SOCKET_CAN_H_

#include <linux/can.h>
#include <linux/can/raw.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include <net/if.h>
#include <arpa/inet.h> 

#include <pthread.h>

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <math.h>
#include <string.h>

static const long DEFAULT_CAN_TIMEOUT = 0; // microseconds.

class CANDevice {
public:
    CANDevice(const char *ifn):iface_name(ifn) {}
    int begin();
    void set_timeout(long timeout_us);
    int write(const can_frame &f);
    int available(); // Check if messages are available
    int read(can_frame &f);
    int set_filter(const struct can_filter &filter);
    int set_filter(uint32_t can_id, uint32_t can_mask);

private:
    int s; // Socket
    const char *iface_name;
    long timeout_us;
};

#endif