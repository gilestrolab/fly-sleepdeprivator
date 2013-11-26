#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
#  sleepDeprivator.py
#  
#  Copyright 2012 Giorgio Gilestro <giorgio@gilest.ro>
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#  
#  

from DAMrealtime import SDrealtime
from time import sleep
from datetime import datetime as dt
import serial, optparse
__version__ = '0.2'

LAG = 5
BAUD = 57600
DEFAULT_PORT = '/dev/ttyACM0'

def start(path, use_serial=True, port=DEFAULT_PORT, baud=BAUD):
    """
    Sleep deprivation routine
    """

    if use_serial: 
        ser = serial.Serial(port, BAUD)
        sleep(2)

    r = SDrealtime(path=path)

    for fname in r.listDAMMonitors():
        command = r.deprive(fname)
        print command
        if command and use_serial:
            print '%s - Sent to Serial port %s' % (dt.now(), port)
            ser.write(command)
        else:
            print '%s - Nothing to send to serial port' % dt.now()

    if use_serial: ser.close()

if __name__ == '__main__':


    usage =  '%prog [options] [argument]\n'
    version= '%prog version ' + str(__version__)

    parser = optparse.OptionParser(usage=usage, version=version )
    parser.add_option('-p', '--port', dest='port', metavar="/dev/ttyXXX", default=DEFAULT_PORT, help="Specifies the serial port to which the SD is connected. Default /dev/ttyACM0")
    parser.add_option('-d', '--d', dest='path', metavar="/path/to/data/", help="Specifies the path to the monitor to be sleep deprived. If a folder is given, all monitors inside will be sleep deprived.")
    parser.add_option('--simulate', action="store_false", default=True, dest='use_serial', help="Simulate action only")
    parser.add_option('--daemon', action="store_true", default=False, dest='daemon_mode', help="Run in daemon mode (continously every 5 minutes)")

    (options, args) = parser.parse_args()

    if options.daemon_mode and options.path:
        print "Starting daemon mode"
        print "Running every %s minutes. Enter Ctrl-C to terminate." % LAG
        while True:
            start(options.path, options.use_serial, options.port)
            sleep(LAG*60)
    elif options.path and not options.daemon_mode:
        start(options.path, options.use_serial, options.port)
    else:
        parser.print_help()
