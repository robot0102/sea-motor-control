#!/usr/bin/python3

import odrive
import time
from odrive.utils import dump_errors
import numpy as np

odrv0 = odrive.find_any()
print("Found motor!")
print(str(odrv0.vbus_voltage))


# Axis states
AXIS_STATE_IDLE = 1
AXIS_STATE_STARTUP_SEQUENCE = 2
AXIS_STATE_FULL_CALIBRATION_SEQUENCE = 3
AXIS_STATE_MOTOR_CALIBRATION = 4
AXIS_STATE_SENSORLESS_CONTROL = 5
AXIS_STATE_ENCODER_INDEX_SEARCH = 6
AXIS_STATE_ENCODER_OFFSET_CALIBRATION = 7
AXIS_STATE_CLOSED_LOOP_CONTROL = 8


# Control Modes
CTRL_MODE_POSITION_CONTROL = 3
CTRL_MODE_VELOCITY_CONTROL = 2
CTRL_MODE_CURRENT_CONTROL = 1
CTRL_MODE_VOLTAGE_CONTROL = 0

# MOTOR types
MOTOR_TYPE_GIMBAL = 2
MOTOR_TYPE_HIGH_CURRENT = 0


def configure_axis(axis):
    # Brake resistance
    odrv0.config.brake_resistance = 0.47 # Ohms

    # Encoder
    axis.encoder.config.cpr = 4000
    axis.encoder.config.use_index = True
    axis.encoder.config.calib_range = 0.1
    axis.encoder.config.idx_search_unidirectional = True

    # Motor Configuration
    axis.motor.config.pole_pairs = 11
    axis.motor.config.calibration_current = 15
    axis.motor.config.current_lim = 15.0

    # Controller configuration
    axis.controller.config.control_mode = CTRL_MODE_POSITION_CONTROL
    #axis.controller.config.control_mode = CTRL_MODE_CURRENT_CONTROL
    # axis.controller.config.vel_limit = 23840000
    # axis.controller.config.control_mode = CTRL_MODE_VELOCITY_CONTROL #CTRL_MODE_VELOCITY_CONTROL
    # axis.controller.config.vel_integrator_gain = 0.0

    # Set needed startup procedures to true
    axis.config.startup_motor_calibration = False
    axis.config.startup_encoder_index_search = False
    axis.config.startup_encoder_offset_calibration = False
    axis.config.startup_closed_loop_control = True
    axis.config.startup_sensorless_control = False

    # Set the direction of index search.
    axis.config.calibration_lockin.vel = 40
    axis.config.calibration_lockin.accel = 20
    axis.config.calibration_lockin.ramp_distance = 3.1415927410125732

    # CAN Bus settings
    axis.config.can_heartbeat_rate_ms = 1000

def calibrate_axis_and_enable(axis):
    # Request calibration
    axis.requested_state = AXIS_STATE_FULL_CALIBRATION_SEQUENCE

    # Wait for calibration to end
    while (axis.current_state != AXIS_STATE_IDLE):
        time.sleep(0.5)

    # Request closed loop control mode.
    print("Calibration done. Setting to closed loop control mode.")
    axis.requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL

configure_axis(odrv0.axis1)
configure_axis(odrv0.axis0)

odrv0.save_configuration()

print("Rebooting the motor. Please wait ....")

try:
    odrv0.reboot()
except:
    pass

odrv0 = odrive.find_any()
print("Found the motor again. Ready to go.")


calibrate_axis_and_enable(odrv0.axis1)
calibrate_axis_and_enable(odrv0.axis0)


dump_errors(odrv0)

# for i in range(5):
#     odrv0.axis0.controller.pos_setpoint = 5000
#     time.sleep(0.8)
#     odrv0.axis0.controller.pos_setpoint = 0
#     time.sleep(0.8)


# for i in range(5):
#     print("Sleeping...")
#     time.sleep(1)


# axis = odrv0.axis1

# for i in range(5):
#     axis.controller.move_incremental(20000, False)
#     time.sleep(5)
#     axis.controller.move_incremental(-20000, False)
#     time.sleep(5)
