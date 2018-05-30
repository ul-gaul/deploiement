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
from numpy import array, linspace, polyfit, poly1d
from sympy import Symbol, expand


# flight step delimiters
launchpad_start = 11351     # timestamp = 1135062
burnout_start = 11512       # timestamp = 1152062
predrogue_start = 11724     # timestamp = 1172178
premain_start = 11757       # timestamp = 1175762
drift_start = 12603         # timestamp = 1260062
landed_start = 13153        # timestamp = 1315062
simulation_end = 13303      # timestamp = 1330062

# flight step timestamp offsets
burnout_offset = 1152062 - 1135062
predrogue_offset = 1172178 - 1152062
premain_offset = 1175762 - 1172178
drift_offset = 1260062 - 1175762

print("burnout offset =", burnout_offset)
print("predrogue offset =", predrogue_offset)
print("premain offset =", premain_offset)
print("drift offset =", drift_offset)


if __name__ == '__main__':    
    timestamp = []
    altitude = []
    csvfilename = 'vol_2017.csv'
    with open(csvfilename, 'r', encoding='ISO-8859-15') as csvfile:
        plots = csv.reader(csvfile, delimiter = ',')
        next(plots)     # skip header
        for row in plots:
            timestamp.append(int(row[1]) - 1135062)
            altitude.append(float(row[2]))
    
    # build numpy arrays of data
    array_launchpad = array(list(zip(timestamp[launchpad_start:burnout_start], 
                                     altitude[launchpad_start:burnout_start])))
    array_burnout = array(list(zip([t - burnout_offset for t in timestamp[burnout_start:predrogue_start]], 
                                     altitude[burnout_start:predrogue_start])))
    array_predrogue = array(list(zip([t - predrogue_offset for t in timestamp[predrogue_start:premain_start]], 
                                     altitude[predrogue_start:premain_start])))
    array_premain = array(list(zip([t - premain_offset for t in timestamp[premain_start:drift_start]], 
                                     altitude[premain_start:drift_start])))
    array_drift = array(list(zip([t - drift_offset for t in timestamp[drift_start:landed_start]], 
                                     altitude[drift_start:landed_start])))
    
    # compute fitting polynomial curves
    xlaunchpad = array_launchpad[:,0]
    ylaunchpad = array_launchpad[:,1]
    zlaunchpad = polyfit(xlaunchpad, ylaunchpad, 1)
    curvelaunchpad = poly1d(zlaunchpad)
    xfitlaunchpad = linspace(xlaunchpad[0], xlaunchpad[-1], 50)
    yfitlaunchpad = curvelaunchpad(xfitlaunchpad)
    
    xburnout = array_burnout[:,0]
    yburnout = array_burnout[:,1]
    zburnout = polyfit(xburnout, yburnout, 3)
    curveburnout = poly1d(zburnout)
    xfitburnout = linspace(xburnout[0], xburnout[-1], 50)
    yfitburnout = curveburnout(xfitburnout)
    
    xpredrogue = array_predrogue[:,0]
    ypredrogue = array_predrogue[:,1]
    zpredrogue = polyfit(xpredrogue, ypredrogue, 3)
    curvepredrogue = poly1d(zpredrogue)
    xfitpredrogue = linspace(xpredrogue[0], xpredrogue[-1], 50)
    yfitpredrogue = curvepredrogue(xfitpredrogue)
    
    xpremain = array_premain[:,0]
    ypremain = array_premain[:,1]
    zpremain = polyfit(xpremain, ypremain, 1)
    curvepremain = poly1d(zpremain)
    xfitpremain = linspace(xpremain[0], xpremain[-1], 50)
    yfitpremain = curvepremain(xfitpremain)
    
    xdrift = array_drift[:,0]
    ydrift = array_drift[:,1]
    zdrift = polyfit(xdrift, ydrift, 1)
    curvedrift = poly1d(zdrift)
    xfitdrift = linspace(xdrift[0], xdrift[-1], 50)
    yfitdrift = curvedrift(xfitdrift)

    
    # trim useless data
    flight = plt.figure(1)
    plt.plot(timestamp[launchpad_start:simulation_end], 
             altitude[launchpad_start:simulation_end])
    plt.xlabel('Timestamp')
    plt.ylabel('Altitude (m)')
    plt.title('Altitude through time')
    flight.show()
    
    launchpad = plt.figure(2)
    plt.plot(xlaunchpad, ylaunchpad, 'o', xfitlaunchpad, yfitlaunchpad)
    plt.xlabel('Timestamp')
    plt.ylabel('Altitude (m)')
    plt.title('Altitude through time during launchpad step')
    launchpad.show()
    
    burnout = plt.figure(3)
    plt.plot(xburnout, yburnout, 'o', xfitburnout, yfitburnout)
    plt.xlabel('Timestamp')
    plt.ylabel('Altitude (m)')
    plt.title('Altitude through time during burnout step')
    burnout.show()
    
    predrogue = plt.figure(4)
    plt.plot(xpredrogue, ypredrogue, 'o', xfitpredrogue, yfitpredrogue)
    plt.xlabel('Timestamp')
    plt.ylabel('Altitude (m)')
    plt.title('Altitude through time during predrogue step')
    predrogue.show()
    
    premain = plt.figure(5)
    plt.plot(xpremain, ypremain, 'o', xfitpremain, yfitpremain)
    plt.xlabel('Timestamp')
    plt.ylabel('Altitude (m)')
    plt.title('Altitude through time during premain step')
    premain.show()

    drift = plt.figure(6)
    plt.plot(xdrift, ydrift, 'o', xfitdrift, yfitdrift)
    plt.xlabel('Timestamp')
    plt.ylabel('Altitude (m)')
    plt.title('Altitude through time during drift step')
    drift.show()
    
    t = Symbol('t')
    print("Équation launchpad:\n", expand(curvelaunchpad(t)))
    print("Équation burnout:\n", expand(curveburnout(t)))
    print("Équation predrogue:\n", expand(curvepredrogue(t)))
    print("Équation premain:\n", expand(curvepremain(t)))
    print("Équation drift:\n", expand(curvedrift(t)))

    input("Press enter to end")
