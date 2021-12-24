#!/usr/bin/python3

import numpy as np
import serial

if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyS0') # Open the serial port
    