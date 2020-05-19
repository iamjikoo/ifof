IFoF 
=======

IFoF 
--------


This project is a simple monitor program for the IFoF Capstone project on the STM32L432 board. 
This processor has 256K of flash and 48K of RAM. The board has 10 LEDs, sensers.

The simple monitor prgram provides a command line parser, with facilities for the user to add new commands on a module by module basis. The parser is capable of recognizing one word commands, and calling a function that implements that command. The function may then retrieve additional arguments from the command line, and act accordingly. Online brief help is provided automatically, extended help is available optionally on a command by command basis. 

The structure of the simple monitor program is one of a foreground/background system. The main loop of the program initializes all the peripherals, enables interrupts, sets up I/O conditions, and then calls a series of 'Tasks' in sequence. The user can extend this by adding their own background tasks to the main loop. Caution should be exercised by the user to ensure that every task returns back to the main loop if there is no work to do. Any task that blocks waiting for an I/O event to occur will stall the whole system. (For example, calling the delay() function inside your task is frowned upon, use a timer or virtual timer instead.)

Note: There are multiple STM32 board versions out there.
The Rev B. boards come preloaded with STLINK-V2, whereas the Rev C. boards have STLINK-V2-1. openOCD cannnot automatically determine the difference. This means that in the Makefile, if you have a Rev B. board, you will have to uncomment the relevant lines before being able to program it. The default is Rev C now.

Installation
-------------

Open an Ubuntu terminal window by pressing CTRL-ALT-T.


From the command prompt execute the following:

    sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
    sudo apt-get update
    sudo apt-get install gcc-arm-none-eabi

Install:

    sudo apt-get install libftdi-dev libusb-1.0-0-dev

 3. You now need to install the On chip debugger support. You should be able to find the file in the tools folder. Download it directly into ubuntu and expand the file into the directory structure which will contain a directory called openocd-0.9.0. Go into the directory you expanded it to and type:

     ./configure

 4. Then in this directory type "make"

 5. Then in this directory type "sudo make install"

 6. You can test if the openOCD install worked by running the following command:

    /usr/local/bin/openocd --version
 
 7. To enable st-link support you might have to run the openocd command:

    sudo ./configure --enable-stlink

 8. To install minicom perform the following:

    sudo apt-get install minicom
 
 9. To run minicom with your USB cables connected to the STM32 board run. 

    sudo minicom -D /dev/ttyACM0
 
The following steps if done correctly will make it so yo do not have to put in sudo all the time for the make commands and minicom.

  1. Minicom accessing the serial port. Initial default configuration of minicom must be done as root still though. The user that they are using must be in the 'dialout' group (logginname is the name you used when you created the ubuntu install):
     usermod -a -G dialout loginuser
 
Two things need to happen, the user must be in the plugdev group. and the permissions on the USB device must be set correctly. The udev daemon is responsible for setting the file permissions, and in order to it correctly, a config file must be loaded in to the udev config directory. I'm not curtain of the name for the file, but it's the only '.rules' file in the contrib directory. Again loginname is the user name you created the ubutu install with.
    usermod -a -G plugdev loginname
    cp /usr/local/share/openocd/contrib/99-openocd.rules /etc/udev/rules.d/

2. Avoiding the '~~x~~x~~' and the ttyACM0 port being busy for 30 sec after attaching. This happens because of a daemon called 'modemmanager' attemting to configure the virtual com port as a modem. If we remove the package, we get rid of the problem. 
    apt-get remove modemmanager

After all of those steps, the user should log out and log back in (to get their userid properly into both groups) and they should completely unplug and plug back in the STM32 if they had it plugged in before. (To refresh the file permissions on the USB device) A single 'sudo make clean' might need to be issued in the build directory, if they had previously compiled the code with sudo. 
