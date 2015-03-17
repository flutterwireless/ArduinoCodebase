### Flutter Arduino Port
This is a modified version of the Arduino IDE version 1.5.6-r2. To see which changes have been made, simply run a diff of the files in this folder with the unmodified repository. A copy of that is available here:
www.flutterwireless.com/files/Arduino-1.5.5-r2.zip

We recommend "Meld" as a diff tool if you do not already have one.

### Running this code
To run this code, open a terminal and change directories to the /build folder. Run "ant run". If your system is appropriately configured, that command will cause your system to begin compiling the Arduino IDE and run the program upon successful completion.

If you have errors, you may need to install some tools. A script is included in the root of this repository "setup.sh" that should install these programs. The script was written for a slightly different purpose so it may throw some errors at the end, but hopefully it properly installs the programs. If this fails, check back in a few days and hopefully we will have the script updated for this use.


### Porting notes

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

I'm trying to get basic Arduino functionality on this Sam3s1 asap, so hopefully once I get something working I can just post that. The differences between one working setup and the Due setup should highlight what all needs to be changed in order for the rest of you to finish your ports.

Together, we can bring the power of Arduino to a whole slew of new low cost ARM chips. The chip I am using us much cheaper than the 32u4 and should support USB reprogramming etc. It should make for a low cost Arduino-compatible board with ARM, rather than the $60 Due. I'm building it into a $20 board that also features a long range wireless radio capable of transmitting over a kilometer, for example. I hope to smash the limits on what people can do with Arduino hardware. :)

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



Explanation of Arduino for Engineers:
Arduino is like the easy bake oven of embedded systems. It is designed to put the power of embedded systems in the hands of a regular joe, by eliminating the troubles of new cpu bring-up and letting them get straight to the development of simple application code. If you are an embedded engineer, you are familiar with this concept. If you've wanted to use a CPU from a ATMEL or Microchip, you've usually saved yourself some time by buying a development kit, and using their pre-canned software package. This eliminates costly bring-up at the early stages of a product, when proof-of concept or other higher level development concepts may initially be more important to the project. You save a lot of development time at the beginning of a project with development kits. Eventually, you design a product with your own CPU and toolchain, based on project requirements. But the development kit was a valuable way to trade time for convenience that positively impacted your development.

Regular joe has a project just like you. Only his will not be profitable. Instead he is sacrificing money and time in the hopes of personal enrichment. He cannot afford high end development tools and does not have the luxury of meetings with his team. He just has his evenings.

But he wants the power of embedded systems. You know that really cool robot you built for that aerospace company or that genetics research lab? That was all built with the power of industry. The power of engineering funded by speculation on it's future value. The regular joe has the same interests as you, and is subject to the same constraints. But he lacks the power of industry that you are normally a part of. He can't afford a $300 dev kit and 40 hours a week on perfect code. If he is to build with embedded systems, he must find a cheaper way of benefitting from their use. He can only benefit from embedded systems if using them is within his financial capacity, skill level, and time budget. And he will not make any profit. No one is giving him a paycheck.

So, Enter Arduino. Arduino is like an engineers dev kit, only the hardware is designed for his kind of applications, and the code is all packaged into extremely simple libraries. He can buy a board at Fry's, download the software and plug in the board, click on an example blink program, hit "Program", and watch a program blink. Like a student on the first day of embedded systems class, he is now sitting in front of a powerful embedded system with a simple blink program running. This is far from buying a blinking light from the store. He now has a fully functioning embedded toolchain, with code and docs ready to be consumed.

In order to attain this zero configuration toolchain, some sacrifices have been made. The manufacturer has made libraries available, and they are a bit complex. They sell a few different boards, but they want the users to download one piece of code to program them. So they abstract out a bunch of trivial things, so the code will support any chip in the future. They are a regular company, and they don't want to have to write the code twice. It doesn't "feel" right to an embedded engineer (for valid reasons), but it all mostly compiles away so it's generally alright. Some functions may take a few extra instructions and take longer to run, but our regular Joe is not planning on building any applications that would be sensitive to microsecond delays in his code. An embedded engineer would cringe at hearing that, but he is not building things with the same goals and constraints as our Joe.

Like the project manager at an engineering company, our regular Joe (or perhaps Joelynn), wants an engineering project completed. Your engineering manager wants it done because our competitors are something something and his boss says something something. Your engineering manager wants it because it is his job. Makes sense, we all have jobs, it's good to do them. Joelynn wants it because she wants to build something for herself. We respect people who take that kind of initiative for their lives.

She may want to build a robot with a friend, a james-bond hidden hatch for her keys at home, or a system that texts her phone when the plants on the back patio need watering. She has requirements and constraints, just like any engineering project. Only when companies build a product it has to work perfectly when deployed to thousands of users for 5 years. Any maintenece is extremely costly, consuming thousands of dollars of resources for even a minor patch. It is not an option for you to build it halfway and then pull it from the market if it becomes too hard to maintain. Joelynn doesn't have those constraints - she used to just go outside and water the plans, she's just a bit busy for the next year and wants to make sure the planter still thrives. If her "product" breaks, she can fix 100% of the deployments in ten minutes - it was just a loose wire after all. She also doesn't have as much up-front engineering as a company would have. She can build a prototype, deploy that, and then improve it more in a few months. Joelynn's only "customer" is herself, so her project has the most understanding type of customer you could have. Interruptions can and will be tolerated by the customer, unlike business life. Joelynn does not do engineering management often, so she is not well-adept at determining project requirements and setting budgets. Thus, like anyone else attempting to do their best in an area where they are unfamiliar, she makes a guess on the budget based on an estimate of the relative value of the output compared to her finances and time availability, taking into consideration some guess to what things cost. She's a welder and aspiring writer by trade, so we can't hold it against her that she's not an expert on setting budgets for embedded systers projects.

She has a loose budget of $45, with an option for more "if management deems it prudent". Basically, she might break her childhood piggy bank - she's been meaning to for years. Her time budget is 3 hours a night two nights a week for 3 weeks. That's when her family visits, and she'd like to be able to stop working on the project by then as well as have something to show them. She has a full life already going, so this is a pretty big commitment for her, but she thinks learning it will be worthwhile. So she has $45 and eighteen hours total to build her project. Tight schedule!

The first night, she just does research. Her friend told her about Arduino, so she looked that up. She learned what the Arduino is, what different boards are popular, and how they all vary. She then started to discover how it would be put together. She orders parts at the end of the night - an Arduino Uno, a wall power supply, an LED display, a button, and a moisture sensor. She spends the second night that week on code academy, learning C programming while the parts arrive.

The next week the parts have arrived. After downloading the software, she opens up the Arduino Uno and plugs it into the computer. She opens the example Blink program, selects "Arduino Uno" from the board menu, and hits "Program". It blinks! Engineering on embedded system-bring up is completed. On to Application development and hardware integration! She fiddles with the blink program, finally seeing in real life what she has been learning on the internet. In some way, she "gets" it now. She sees how code on the screen can make something happen in real life. She doesn't know down to the transistor level, but maybe someday she will. For now, she starts looking at the documentation for the LED display. They provide a set of sample applications that demonstrate its capabilities. She uses a few wires to hook up the display as shown in the documentation, then downloads the examples and opens the first one. Then she runs the program, and the display lights up! It prints hello world in glowing blue letters, scrolling it across 4 seven-segmented digits. She changes the code to say "Hello Joelynn" and loads it on. She is rolling along. The next example displays numbers. This is what she needs. She plays with that, finding the section of the example that does the critical parts and copying it into her own program. She writes a program that counts to 1000, increasing in speed as the number goes up. She can see numbers from her program in the real world. That's useful! But then, her night is ending. She'll have to come back to this in two days.

When she comes back, she decides to get the moisture sensor working. She follows that example code, and it prints the numbers to a terminal. She had to use a different computer, and she spent an hour and a half getting serial ports working, due to some different behavior in that version of windows. Getting around this saps her night. She gets the example to run and spit out numbers, dipping the sensor in a mug of water and watching it change onscreen, but that's all she has time for. This is the end of the second week. She only has two nights left before her parents arrive!

The next day, she brings the moisture sensor example back up on her main computer, and sees how she can patch that code into the LED display example. She can make it display the moisture levels on the LED display. She hopes this will be all she needs to help her know when the plants need monitoring. She takes the project outside to the planter, wires hanging off, and places it in the dirt with the display stuck to the outside of her window. This is the project testing phase. If it works, she will be able to do final packaging and "delivery" next time.

In the two days following, hot weather dries out the soil and the number on her display goes down. She waters it at night and the number goes back up. By the time she works on it next, she knows the concept is sound. She spends the evening soldering on some longer wires that are also more permanent than the prototyping wires she was using. She runs the wires to the display inside of a white tube for looks, and puts the Arduino board inside of a tupperware container in the planter to hide it from view and the elements. Her product is deployed! Over the next few weeks, she learns how to use this new tool she has made for herself. She knows that when the sensor goes down to 65 or below, it's dry to the touch. When it says 120 or better, it's nice and moist. A few weeks later she will upgrade it to only display the number when it is low, with a pushbutton to check the number at any time to make sure the system is still working. Her parents are very impressed, and once she's done with the project she takes a much needed break and focuses on hanging out with friends for a little while. And her plants are always watered. The project was a success.
