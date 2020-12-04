# frisquet-RFLink

RFLink tools for Frisquet Eco Radio System boiler / Utilitaires RFLink pour chaudière Frisquet Eco Radio System
Flash RFLink hardware to be able to decode or encode Frisquet Eco Radio System protocol. This will enable remote control of a Frisquet boilers using Eco Radio System.

Tested successfully on models:
- Hydromotrix

# Usage

Using Arduino IDE, load inside RFLink hardware :
- frisquet-ERS-decode to decode ERS protocol and retrieve the ID of your boiler
- frisquet-ERS-command to command your boiler using ERS protocol (don't forget to set the ID of your boiler)
