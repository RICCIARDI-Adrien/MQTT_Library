# MQTT Library
Simple MQTT library tailored for ARM Cortex-M3 like cores with a C library and no malloc() support (even if it can be used with malloc() too).  
Implementation is based on [MQTT specifications version 3.1.1](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html).

## Building
Recommended way is :
* Add the library to your project as a git submodule.
* Add the library directory to you includes path.
* Build MQTT.c and link it to your program.

## Example
An example program running on a PC is provided. It allows to publish data to a standard MQTT server.
