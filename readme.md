### Flutter Arduino Port
This is a modified version of the Arduino IDE version 1.5.5-r2. To see which changes have been made, simply run a diff of the files in this folder with the unmodified repository. A copy of that is available here:
www.flutterwireless.com/files/Arduino-1.5.5-r2.zip

We recommend "Meld" as a diff tool if you do not already have one.

### Running this code
To run this code, open a terminal and change directories to the /build folder. Run "ant run". If your system is appropriately configured, that command will cause your system to begin compiling the Arduino IDE and run the program upon successful completion.

If you have errors, you may need to install some tools. A script is included in the root of this repository "setup.sh" that should install these programs. The script was written for a slightly different purpose so it may throw some errors at the end, but hopefully it properly installs the programs. If this fails, check back in a few days and hopefully we will have the script updated for this use.


### Debugging
See porting_notes.md for notes from early 2014 porting of the project.

