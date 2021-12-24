#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import sys
import yaml
import re
import pandas as pd
import argparse

def extract_logvars(logfile):
    """
    Extract a variable from the SPDlog file
    """
    with open(logfile) as fh:
        records = []
        vars_dict = {}
        for line in fh:
            tmpvar = line.strip().split(' ')
            name  = tmpvar[3][:-1]
            values = tmpvar[4:-1]

            if len(values) == 1:
                values_num = float(values[0])

            if name in vars_dict.keys():
                vars_dict[name].append(values_num)
            else:
                vars_dict[name] = [values_num]

        for k in vars_dict.keys():
            vars_dict[k] = np.array(vars_dict[k])

    return pd.DataFrame(vars_dict)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Converts spdlog files to csv.")
    parser.add_argument('logfile', metavar='logfile', type=str, nargs='+',
                         help='The log file to convert.')
    parser.add_argument('-o', action='store', dest='output_file',
                         help='Name of output file.')

    results = parser.parse_args()

    df = extract_logvars(results.logfile[0])
    df.to_csv('logs/pos_experiment_data.csv')
    tvals = np.arange(0, df.shape[0])/200

    plt.figure(figsize=(8, 6))
    plt.plot(tvals, df['experiment.measured_position'], linewidth=1, 
             label='Measured Position')
    plt.plot(tvals, df['experiment.position_setpoint'], linewidth=1,
             label='Position Setpoint', linestyle='--')
    plt.xlabel('Time (s)')
    plt.ylabel('Position (counts)')
    plt.title('Position Control Plot')
    plt.legend()
    plt.grid()
    plt.savefig('logs/pos_experiment.png', dpi=400)

