#include "SocketCAN.hpp"


int CANDevice::begin()
{
    // Set default timeout
    timeout_us = DEFAULT_CAN_TIMEOUT;
    // Initialize socket
    struct sockaddr_can addr;
    struct ifreq ifr;

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Error while opening socket!!");
        return s;
    }

    strcpy(ifr.ifr_name, iface_name);
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Bind socket
    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Error in socket bind!");
        return -1;
    }

    return 0;
}

void CANDevice::set_timeout(long timeout_us_)
{
    timeout_us = timeout_us_;
}


int CANDevice::available()
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

int CANDevice::write(const can_frame &f)
{
    int nbytes;
    nbytes = ::write(s, &f, sizeof(struct can_frame));

    // Error checking
    if (nbytes < 0) {
        perror("Could not write to socket!\n");
        return nbytes;
    }

    /* paranoid check ... */
    if (nbytes < (int) sizeof(struct can_frame)) {
        perror("Incomplete can frame written.\n");
        return nbytes;
    }

    return nbytes;
}

int CANDevice::read(can_frame &f)
{
    int nbytes;
    nbytes = ::read(s, &f, sizeof(struct can_frame));

    /* paranoid check ... */
    if (nbytes < (int) sizeof(struct can_frame)) {
        printf("ERROR: Read incomplete CAN frame.\n");
    }

    return nbytes;
}

int CANDevice::set_filter(const struct can_filter &filter)
{
    int ret;
    ret = setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));

    return ret;
}

int CANDevice::set_filter(uint32_t can_id, uint32_t can_mask)
{
    can_filter f;
    f.can_id = can_id;
    f.can_mask = can_mask;

    int ret = setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &f, sizeof(f));

    return ret;
}