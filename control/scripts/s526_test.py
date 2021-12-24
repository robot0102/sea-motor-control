#!/usr/bin/python3

import sys
import time
import os
import portio as io
import numpy as np

TIMER_MODE_EDGES = 0b0011010101010100
TIMER_MODE_LEVEL = 0x1540
LATCH_INDEX_DOWN = 0x0400
COUNTER_ARM      = 0x2000
SECONDS_PER_COUNT= 1.0/(27e6)

S526_BASE_ADDRESS = 0xd00

COUNTER_MODE_PWM = 0


class Model526:
    def __init__(self, base_address):
        """
        Setup the Model 526.
        """
        self.base_address = base_address
        ret = io.iopl(3)

        # Initialize register values
        self.REG_TCR = 0x00
        self.REG_WDC = 0x02
        self.REG_DAC = 0x04
        self.REG_ADC = 0x06
        self.REG_ADD = 0x08
        self.REG_DIO = 0x0A
        self.REG_IER = 0x0C
        self.REG_ISR = 0x0E
        self.REG_MSC = 0x10
        self.REG_C0L = 0x12
        self.REG_C0H = 0x14
        self.REG_C0M = 0x16
        self.REG_C0C = 0x18
        self.REG_C1L = 0x1A
        self.REG_C1H = 0x1C
        self.REG_C1M = 0x1E
        self.REG_C1C = 0x20
        self.REG_C2L = 0x22
        self.REG_C2H = 0x24
        self.REG_C2M = 0x26
        self.REG_C2C = 0x28
        self.REG_C3L = 0x2A
        self.REG_C3H = 0x2C
        self.REG_C3M = 0x2E
        self.REG_C3C = 0x30
        self.REG_EED = 0x32
        self.REG_EEC = 0x34

        self.ISR_ADC_DONE = 0x4

        self.IER = 0

        if ret != 0:
            raise RuntimeError("Cannot set iopl(3). Try running script as root.")
    
    def write_register(self, value, addr):
        """
        Write to a register of the model 526.
        """
        io.outw(value, addr + self.base_address)

    def read_register(self, addr):
        """
        Read a register in the Model 526.
        """
        return io.inw(addr + self.base_address)

    def analog_read(self, channel):
        """
        Read an analog channel.
        """
        # Enable the ADC interrupt
        self.IER |= self.ISR_ADC_DONE
        self.write_register(self.IER, self.ISR_ADC_DONE)
        # Reset the interrupt (just being paranoid)
        self.write_register(self.ISR_ADC_DONE, self.REG_ISR)

        # Write configuration to convert and read channel.
        # Also, start conversion.
        adc_cfg = 0xFFE0 | 1
        self.write_register(adc_cfg, self.REG_ADC)

        # Wait for the ISR bit to set (conversion done)
        for i in range(400): # Following the comedi driver
            if self.ISR_ADC_DONE & self.read_register(self.REG_ISR):
                print("ADC Conversion done on check {}".format(i))
                # Reset the interrupt bit
                self.write_register(self.ISR_ADC_DONE, self.REG_ISR)
                break

        # Read the ADC data
        readings = []
        for i in range(10):
            self.write_register(0xFFE0 | (i << 1), self.REG_ADC)
            val = self.read_register(self.REG_ADD)
            #print(val)
            readings.append(val)

        # Return result
        return readings

        

def pwm_write(channel_number, period_ms=10, duty_cyle=0.2):
    """
    Setup counter to do PWM.
    """
    period_count = int(period_ms/1.0e3 *27e6)
    high_count = int(period_count*duty_cyle) & 0xFFFF
    low_count = int(period_count*(1-duty_cyle)) & 0XFFFF
    #print(period_count, hex(high_count), hex(low_count))
    

    # Set the low count in PR0
    print("Setting low_count to {}".format(hex(low_count)))
    io.outw(0x1C85, S526_BASE_ADDRESS + (0x16 + 8*channel_number))
    io.outw((low_count >> 16) & 0xFF, S526_BASE_ADDRESS + (0x14 + 8*channel_number))
    io.outw(low_count & 0xFFFF, S526_BASE_ADDRESS + (0x12 + 8*channel_number))

    # Set the high count in PR1
    print("Setting high_count to {}".format(hex(high_count)))
    io.outw(0x5C85, S526_BASE_ADDRESS + (0x16 + 8*channel_number))
    io.outw((high_count >> 16) & 0XFF, S526_BASE_ADDRESS + (0x14 + 8*channel_number))
    io.outw(high_count & 0xFFFF, S526_BASE_ADDRESS + (0x12 + 8*channel_number))

def analog_read(channel_number):
    """
    Read model 526 analog channel.
    """
    pass

if __name__ == '__main__':
    ret = io.iopl(3)

    s526 = Model526(0xd00)

    for i in range(10):
        print(s526.analog_read(0))
    
    #print(hex(io.inw(0xd34)))

    #pwm_write(0, 0.0005, 0.5)



# if __name__ == '__main__':
#     # Set the io previlige to highest
#     ret = io.iopl(3)
#     print("iopl: ", os.strerror(ret))

#     # Read S526 board id
#     board_id = io.inw(0xd00 + 0x34)
#     print(hex(board_id))
#     if hex(board_id) == '0x526b':
#         print("Sensoray model 526 detected!")

#     # Setup counter 0 mode register
#     io.outw(0x55AC, 0xd00 + 0x16)
#     io.outw(0x0000, 0xd00 + 0x12)
#     io.outw(0x0000, 0xd00 + 0x14)

#     io.outw(0x35AC, 0xd00 + 0x16)
#     io.outw(0x0000, 0xd00 + 0x12)
#     io.outw(0x0000, 0xd00 + 0x14)
#     io.outw(0xC80F, 0xd00 + 0x18)
#     led = 0
    
#     for i in range(1000):
#         # Arm and read the counter
#         #io.outw(0x040F, 0xd00 + 0x18)
#         time.sleep(0.01)
#         lw = io.inw(0xd00 + 0x12)
#         hw = io.inw(0xd00 + 0x14)
#         count = hw<<16 | lw
#         io.outw(led, 0xd00 + 0x10)
#         print(count*SECONDS_PER_COUNT)
#         #print("COUNT: ", count)
#         #print("INDEX: ", io.inw(0xd00 + 0x18)>>5)
#         #print("EVENTS: ", io.inw(0xd00 + 0x18)&0xf)
#         led = led^1
