# Porting Notes
Below are my original porting notes from early 2014. They may prove informative so I will leave them here.

Makefiles:

find:
~/ARM/ArduinoDev/Arduino-1.5.5-r2/hardware $ find . -name Makefile

There are three pieces of code that are compiled together at the time the user hits verify.

These pieces are object files and C code sitting in the /hardware/ folder of the arduino main tree, 1.5.x branch (and others, likely).

The C code is comprised of the user's Arduino application code, which is merged into a "main" file that includes other code, as well as the Arduino libraries invoked in that code. All of that c code is compiled into object code, which is then linked to other object code that was previously compiled. Specifically, there is a file with object code specific to your processor, as well as an object file with codes specific to your hardware.

The code for your processor, code for your hardware board, and user's application code all get compiled into the single file that is loaded onto the user's board.

To port arduino over to a new board or processor, you will need to ensure that the object files are being correctly generated for your target. If you are using the same processor as the Arduino Due, you need only duplicate the arduino_due_x folder inside the variants directory at hardware/arduino/sam/variants/ and then change a handful of files inside this directory and its children. Everything needed by this step is in one way or another described in the Makefile inside of the /build_gcc/ folder. To some extent, you need to understand what the makefile is doing in order to see where the appropriate changes need to be made, but perhaps collectively we can start documenting this so that it is easier for others. You need to name your variant something other than arduino_due_x, so anywhere you see that will need to change. If you are planning on using a different chip, the name of the chip library will change, and you will need to find that in the Makefile. Some of the callouts for the hardware code also depend on what chip you have, so you may need to change some #define statements to reflect your chip name. The main Makefile also calls some sub makefiles in the same directory, and those may need changes as well.

The biggest file that needs changing however is the variant.cpp file and it's variant.h header. These files define the pin layout of your specific hardware. A board may have pin labels like #1-20, but your CPU has spare I/O at a random sample of pin numbers. The variant.cpp file holds the pin map between board labels and system pins. A large array defines pins as digital or analog and calls out their other functions such as serial or PWM. When the user runs a peripheral initialization routine, the Arduino code uses a set of standard definititons to check whether the pins being passed to the initialization function actually have this funcrtionality. If the user specifies pins that do not have the necessary capability, the intialization routine will return an error. Users can test for this error in their code to make sure initialization goes correctly. In the absence of a debugger this kind of sanity check is a way to stop the code if basic errors exist, easing development.

I have not found any documentation for the variant.cpp file, but if you look through the Arduino code for functions used in that file it is possible to determine how it works. I will try to document what I know:

A line "extern const PinDescription g_APinDescription[]=" defines the main pin description matrix.

Ever line in the matrix represents a pin on the Arduino. The pin labeled Pin 0 on the Arduino is the first line of the matrix, Arduino Pin 1 is the next line of the matrix, and so on.

Each line has 10 columns that define the full capability of that pin. A basic digital pin with no perhipheral capability looks like this:
{ PIOB, PIO_PB26, ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT, PIN_ATTR_DIGITAL, NO_ADC, NO_ADC, NOT_ON_PWM,  NOT_ON_TIMER },

In order, the attributes describe:
1) The bank of PIO the pin is attached to, such as PIOA, PIOB, or PIOC.
2) The specific pin name, in the above case PIO_PB26.
3) The Peripheral ID for the PIO controller.
4) The type of PIO pin. Either PIO_INPUT, PIO_OUTPUT_0, PIO_PERIPH_A, PIO_PERIPH_B, etc.
5) The primary function of this logical Arduino pin number (PIO_DEFAULT in most cases. used to define a second function for a pin as a higher Digital IO pin number it seems).
6) A Pin attribute identifier, usuall PIN_ATTR_DIGITAL, PIN_ATTR_ANALOG, or a combination or'd together, such as (PIN_ATTR_DIGITAL|PIN_ATTR_TIMER) or (PIN_ATTR_DIGITAL|PIN_ATTR_PWM).
7) The ADC pin name, such as ADC0, ADC12, etc, or NO_ADC if not applicable.
8) An alternate ADC pin name, following the same scheme from above. Not sure the specifics of this. Seems to be an alternate function.
9) The PWM pin name, such as PWM_CH6, or NOT_ON_PWM otherwise.
10) The Timer/Counter pin name, such as TC0_CHB2 or TC0_CHA1, or NOT_ON_TIMER otherwise.


The specific pin names are all sitting in a header file in CMSIS (naturally. the compiler has to know what that stuff means). A quick search of the filesystem for the pin name PIO_PB26 reveals how we can find what all the pin names are. A search for that name finds the header file:
arduino/sam/system/CMSIS/Device/ATMEL/sam3u/include/pio/pio_sam3u1e.h

We see that all of the chips have their own similar definitions under the arduino/sam/system/CMSIS/Device/ATMEL/ folder, and if we are porting to a new chip or just need to see what these pin names are, this is where to look.

Armed with the appropriate pin definitions, we can start filling out the g_APinDescription[] array. I made a simple array first where I only defined about 5 pins. I think if I don't try to use anything above that it will work out alright, but eventually I need to fully define my board there.

There are also some definitions for initialization routines in the variant.cpp file, and those need to be changed. The SAM3s does not have as many pins or peripherals as the Due, so I found that I had to remove a lot of the definitions. There may be startup routines for Serial1, Serial2, and Serial3, but your chip may not have all those definitions. You will need to know what your chip has and arrange these appropriately. You may have to give new names and write new startup routines for hardware your chip has that other's don't. This all takes some time to do.

Once you think you have made appropriate changes, you should see if the code compiles. Actually, you should see if everything compiles before trying to change the variant.cpp file at all. After copying the arduino_due_x folder and its contents, you will need to change a few names and get it compiling again. Once you have a copy of the due code that compiles with your new board's name, you can start looking at modifying variant.cpp and variant.h.

Once your variant builds, you should look at the boards.txt file in the variants folder and add your board, making changes where needed. If you did everything perfectly, you could now compile the Arduino IDE and your board would be integrated into it like any other Arduino. You likely did not do everything perfectly however, and debugging what happened requires some time and skill.

I use a hardware debugging probe (Segger J-Link) and GDB to load on my Arduino code and step through it. Configuring this setup was not trivial, but I can describe it in the future if anyone needs me to. It required snagging a .gdb script from a folder in CMSIS and pointing it to the .elf file generated when I run the Arduino IDE and compile my program (find it by turning on verbose compilation in preferences). I later set up a main file that I can compile from the command line with a Makefile to speed development.

To build a fully working copy of code for any board and chip combo, you ultimately need to compile 3 makefiles. The Libsam is the chip library, and its Makefile is located here:
/hardware/arduino/sam/system/libsam/build_gcc/

Then you need to compile your variant, which will also link in the chip files.
/hardware/arduino/sam/variants/arduino_due_x/build_gcc/
where "arduino_due_x" would be replaced with your variant name.

Lastly, you need to compile in your Arduino application, which can be done from the Arduino GUI by going to the /build/ directory in the root of the source and running 'ant' and then 'ant run' from the command line (in linux).
I also have a makefile to compile in a test application without the GUI, so I can debug my changes quickly. I will share this when I get my code in sharable condition.

Would it be possible for someone who knows more about variant.cpp to try to describe what its contents are? Even if it's a broad overview, I'd love to know what all needs to be in there. Though it's not too terribly difficult to figure out.

Is there a place in the wiki we could start documenting our findings on this? I would really love to see more chips supported!

locations:
cd ~/ARM/ArduinoDev/Arduino-1.5.5-r2/hardware/arduino/sam/system/libsam/build_gcc/

cd ~/ARM/ArduinoDev/Arduino-1.5.5-r2/hardware/arduino/sam/variants/flutter/build_gcc/

cd ~/ARM/ArduinoDev/Arduino-1.5.5-r2/hardware/arduino/sam/cores/arduino/validation/build_gcc/





./arduino/sam/variants/flutter/build_gcc/Makefile
./arduino/sam/variants/arduino_due_x/build_gcc/Makefile

./arduino/sam/cores/arduino/validation_usb_device/build_gcc/Makefile
./arduino/sam/cores/arduino/validation/build_gcc/Makefile

./arduino/sam/system/CMSIS/Examples/cmsis_example/gcc_arm/Makefile
./arduino/sam/system/CMSIS/Examples/cmsis_example/gcc_atmel/Makefile
./arduino/sam/system/CMSIS/Examples/cmsis_example/gcc/Makefile

./arduino/sam/system/libsam/build_gcc/Makefile


./arduino/avr/bootloaders/lilypad/src/Makefile
./arduino/avr/bootloaders/stk500v2/Makefile
./arduino/avr/bootloaders/caterina-Arduino_Robot/Makefile
./arduino/avr/bootloaders/bt/Makefile
./arduino/avr/bootloaders/caterina-LilyPadUSB/Makefile
./arduino/avr/bootloaders/atmega/Makefile
./arduino/avr/bootloaders/atmega8/Makefile
./arduino/avr/bootloaders/optiboot/Makefile
./arduino/avr/bootloaders/caterina/Makefile

 
