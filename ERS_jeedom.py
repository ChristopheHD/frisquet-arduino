#!/usr/bin/env python3
# argv[1] is the temperature we want in percent
# argv[2] is the mode in french : Eco, Confort, Hors gel

import sys
import time
import serial

puissance = float(sys.argv[1])
temperature = 0
if puissance > 0:
	temperature = 20+0.7*puissance

mode = sys.argv[2]
modes = { "Eco":"0", "Confort":"3", "Hors gel":"4"}

command = "ERS " + modes.get(mode, "0") + " " + str(int(temperature)) + "\n\r"

arduino = serial.Serial(port="/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0", baudrate=57600, timeout=1, writeTimeout=1)
time.sleep(1)
arduino.write(command.encode('utf-8'))
arduino.close()
