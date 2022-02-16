#!/usr/bin/env python3
# argv[1] is the temperature we want in percent (min 23 - max 100)
# argv[2] is the mode in french : Eco, Confort, Hors gel

import sys
import time
import serial

puissance = sys.argv[1]
temperature = 0
if puissance > 0:
	temperature = 20+0.7*float(puissance)

mode = sys.argv[2]
modes = { "Eco":"0", "Confort":"3", "Hors gel":"4"}

command = "ERS " + modes.get(mode, "0") + " " + str(int(temperature)) + "\n\r"

arduino = serial.Serial(port="/dev/ttyUSB1", baudrate=57600, timeout=1, writeTimeout=1)
time.sleep(1)
arduino.write(command.encode('utf-8'))
arduino.close()
