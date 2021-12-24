#!/usr/bin/python3

import can

# Setup the bus
bus = can.interface.Bus(bustype='socketcan', channel='can0', bitrate='1000000')

# Simple logging
for msg in bus:
    print(msg)