# AMTW4-Bootloading
The code for the Advanced Microcontroller Topics Workshop #4: Bootloaders.

This project is split into two parts, both built with ```ninja```: the bootloader, and the target program. The bootloader code is in the project root. The target program code is under ```program/```. To build either, navigate to their directory, and run ```./configure.py```, followed by ```ninja```.

There are two linker scripts in this project. The one for the bootloader specifies a 4096-byte region at the bottom of the flash for the bootloader code, and all ```.text``` sections are placed there. For the target program, the linker script specifies the bottom 4096-byte region of the flash as being part of the bootloader, and the rest of the flash as being part of the target program. The target program's ```.text``` sections are placed into the latter.

To bootload the device, first burn the bootloader with ```./program.sh -p``` from the bootloader root. Then, to bootload the device, run ```program/bootload.py <serial port> <baud rate> <firmware image>```. The ```ninja``` build script for the target program specifies that the ```.elf``` file is ```objcopy```'d to a ```.bin``` file, which can be specified as the ```<firmware image>``` argument to ```bootload.py```. Finally, once ```bootload.py``` has opened the serial port to the target, press the reset button on the target to reboot back into the bootloader and start the upload process. You should see ```bootload.py``` spit out a series of messages about each of the flash regions programmed, followed by ```Done!``` on success.

Enjoy!
