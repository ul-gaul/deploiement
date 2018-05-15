#!/usr/bin/env python3

"""
This file takes a CSV file for input containing altitude values of a rocket 
flight and returns mathematical functions representing the altitude for the 
different flight steps.
"""

# imports
import os
import sys
import csv
import matplotlib.pyplot as plt


# flight step delimiters
lauchpad_start = 1135000
burnout_start = 1152090
predrogue_start = 1172178
premain_start = 1175662
drift_start = 1260062
landed_start = 1315062
simulation_end = 1330000

if __name__ == '__main__':
    if len(sys.argv) != 2 or sys.argv[1] == '-h':
        print("Usage: python3 simulate_altitude.py [inputfile]")
        sys.exit(0)
    timestamp = []
    altitude = []
    csvfilename = sys.argv[1]
    with open(csvfilename, 'r', encoding='ISO-8859-15') as csvfile:
        plots = csv.reader(csvfile, delimiter = ',')
        next(plots)     # skip header
        for row in plots:
            timestamp.append(int(row[1]))
            altitude.append(float(row[2]))
    plt.plot(timestamp, altitude, label='Altitude')
    plt.xlabel('Timestamp')
    plt.ylabel('Altitude (m)')
    plt.title('Altitude through time')
    plt.show()
