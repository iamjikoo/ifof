This README is just a fast *quick start* document for the project. 

What is IFoF ?
===============

The IFoF represents  Identification Friend or Foe (IFoF) System which is identify who is our side and who is enomy. This system is used in the aircraft, military forces etc. It is a radar-based identification system, contains a transponder that receives a signal and then sends a response that identifies the broadcastger. 
The system only identifies the friendly targets ignoring the others.
The system sends a notification when a friendly aircraft is detected.

Components
--------------

These compnents used to build the system:

 * Nucleo32-L432KC (STM32L432KC)
 * Laser Diode (TTL Laser Diode) - Converts electrical energy to light energy
 * NRF Module (nRF24L01) - Wireless transceiver module
 * Pulse Sensor  - a plug-and-play heart-beat sensor
 * LCD Display 


Installation
-------------

Open an Ubuntu terminal window by pressing CTRL-ALT-T.


From the command prompt execute the following:

    sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
    sudo apt-get update
    sudo apt-get install gcc-arm-none-eabi

Install:

    sudo apt-get install libftdi-dev libusb-1.0-0-dev

You now need to install the On chip debugger support. You should be able to find the file in the tools folder. Download it directly into ubuntu and expand the file into the directory structure which will contain a directory called openocd-0.9.0. Go into the directory you expanded it to and type:

     ./configure

Then in this directory type "make"

Then in this directory type "sudo make install"

You can test if the openOCD install worked by running the following command:
 ```
/usr/local/bin/openocd --version
 ```
To enable st-link support you might have to run the openocd command:

    sudo ./configure --enable-stlink

To install minicom perform the following:

    sudo apt-get install minicom
 
To run minicom with your USB cables connected to the STM32 board run. 

    sudo minicom -D /dev/ttyACM0
 
The following steps if done correctly will make it so yo do not have to put in sudo all the time for the make commands and minicom.

Minicom accessing the serial port. Initial default configuration of minicom must be done as root still though. The user that they are using must be in the 'dialout' group (logginname is the name you used when you created the ubuntu install):

     usermod -a -G dialout loginuser
 
Two things need to happen, the user must be in the plugdev group. and the permissions on the USB device must be set correctly. The udev daemon is responsible for setting the file permissions, and in order to it correctly, a config file must be loaded in to the udev config directory. I'm not curtain of the name for the file, but it's the only '.rules' file in the contrib directory. Again loginname is the user name you created the ubutu install with.

    usermod -a -G plugdev loginname
    cp /usr/local/share/openocd/contrib/99-openocd.rules /etc/udev/rules.d/

Avoiding the '~~x~~x~~' and the ttyACM0 port being busy for 30 sec after attaching. This happens because of a daemon called 'modemmanager' attemting to configure the virtual com port as a modem. If we remove the package, we get rid of the problem. 

    apt-get remove modemmanager

After all of those steps, the user should log out and log back in (to get their userid properly into both groups) and they should completely unplug and plug back in the STM32 if they had it plugged in before. (To refresh the file permissions on the USB device) A single 'sudo make clean' might need to be issued in the build directory, if they had previously compiled the code with sudo. 
