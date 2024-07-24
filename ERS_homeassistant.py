#!/usr/bin/env python3
# puissance is the temperature we want in percent
# mode is the home assistant mode : eco, comfort, frost

import sys
import asyncio
import serial

@service
def ERS_set_temp(puissance=0, mode="comfort"):
    temperature = 0
    log.info(f"ERS puissance {puissance} in mode {mode}")
    if puissance > 0:
        temperature = 20+0.7*puissance
    
    modes = { "eco":"0", "comfort":"3", "frost":"4"}

    command = "ERS " + modes.get(mode, "4") + " " + str(int(temperature)) + "\n\r"

    arduino = serial.Serial(port="/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0", baudrate=57600, timeout=1, writeTimeout=1)
    asyncio.sleep(1)
    arduino.write(command.encode('utf-8'))
    arduino.close()
          
