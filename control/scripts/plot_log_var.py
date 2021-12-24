#!/usr/bin/python3

import matplotlib.pyplot as plt
import numpy as np

pos_estimate = np.loadtxt('logs/experiment.measured_position.csv')
vel_estimate = np.loadtxt('logs/experiment.measured_velocity.csv')
pos_setpoint = np.loadtxt('logs/experiment.position_setpoint.csv')
delta_t = np.loadtxt('logs/delta_t.csv')

ST = 0
END = 4000
pos_estimate = pos_estimate[ST:END]
vel_estimate = vel_estimate[ST:END]
pos_setpoint = pos_setpoint[ST+1:END+1]
delta_t = delta_t[ST:END]

time = np.arange(0, np.shape(pos_estimate)[0])/200.0

plt.figure(figsize=(9, 14))
plt.subplot(3, 1, 1)
plt.title('Position Estimate')
plt.plot(time, pos_estimate, label='Measured Position')
plt.plot(time, pos_setpoint, label='Position Setpoint')
plt.ylabel('Position (counts)')
plt.xlabel("Time (s)")
plt.grid()
plt.legend()
plt.subplot(3, 1, 2)
plt.plot(time, vel_estimate)
plt.title('Velocity Estimate')
plt.ylabel('Velocity (counts/sec)')
plt.xlabel('Time (s)')
plt.grid()
plt.subplot(3, 1, 3)
plt.plot(time, delta_t)
plt.title('Loop time')
plt.ylabel("Time (ms)")
plt.xlabel("Time (s)")
plt.grid()
plt.tight_layout()
plt.savefig('logs/posvel.png')
