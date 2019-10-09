# NIProtocol

This repository contains the code necessary to do two things:

1. Read button/wheel input from the Native Instruments Maschine Mikro 2.
2. Send button input from an iPhone to the Native Instruments Maschine 2 application.

NOTE: All of the code is written for MacOS. Significant changes will be needed for this code to work on other operating systems.

## Reading the MK2

The Maschine Mikro 2 (MK2) interfaces with your pc through the Native Instrument drivers. The drivers then forward the data to the NIHardwareAgent (at least on MacOS) which in turns forwards it to the Maschine 2 GUI.

The `m2client` program, which can be built by running `make` while inside the `client` directory, will act as the Maschine 2 GUI and interact with the NIHardwareAgent.

When `m2client` is run, it will perform the handshake with the hardware agent and create a `CFMessagePortRef` with name "SIHWMainHandler" for listening applications to send messages to the MK2 through.

### Performing the handshake

To perform the handshake (and build the client), multiple modules first need to be built:

1. `niparser`: This module will parse the wheel or button packets from the hardware agent into structs.
2. `nimessenger`: This module sends the messages necessary for performing the handshake and interacting with the MK2 (such as setting button LED's) to the hardware agent.
3. `ninotifier`: This module implements the functions necessary for program to register as a listener for incoming packets and for a program to broadcast a packet to any listening program.
4. `nihandshaker`: This module contains the functionality to perform the handshake from the client side with the hardware agent. The previous three modules need to be built in order for this module to be built.

These can all be built using the `make_libs.sh` script or each can be built individually with the Makefile in its respective directory.

Note that when each library is built, its header and compiled dylib will be copied into the `usr/local/include` and `usr/local/lib` directories respectively so that they can be included through system paths.

After `m2client` performs the handshake, when a new packet is forwarded from the hardware agent, it will then be parsed into a structure and broadcast via a `CFNotification` to any listening program.

### Listening to the notification

Any program can use the `ninotifier` module to listen to the broadcasted packets. There is an example application in the `testapp` directory that registers itself as a listener and prints the pressed button or wheel action when a new packet is received.

## Writing to Maschine 2

The second part of this project is to act as the hardware agent and send packets to the Maschine 2 GUI. The `m2server` program (located in the `server` directory) will perform that task. The server program relies on the same four modules that the client is dependent on.

`m2server` will also set up a TCP socket on port 6969 to listen for button commands from the `Maschine 2` iPhone application. It will then register a Bonjour service with name `M2` on this port. The `Maschine 2` application, when run, will connect to the published Bonjour service and connect to port 6969.

Currently, pressure sensitivity is not available in the `Maschine 2` app since my phone does not support 3D Touch.