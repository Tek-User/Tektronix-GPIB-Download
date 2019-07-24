# Tektronix-GPIB-Download
Software (and hardware) to allow copying your oscilloscope display directly to a flash drive

This purpose of this project is to create an inexpensive, and simple-to-build device which attaches to Tektronix 2430, 2430A, 2432, and 2440 digital storage oscilloscopes to allow the user to download the displayed trace(s) to a flash drive without the need for a computer to be attached to the oscilloscope.  No hardware beyond the device and a GPIB cable are required.  The downloaded file on the flash drive is to be a vector graphics file type which can be viewed and printed using readily available free software.  

An Arduino (or clone) Mega2560 is required as the smaller devcices do not have enough I/O pins to do the job properly.

The program interfaces to the oscilloscope front panel user-interface buttons.  It will save the displayed traces to a SVG file along with the volts/div and sec/div settings.  All 1024 data points are stored in the vector graphics file.

Control menus allow storing the trace(s), setting date/time/filename.

About 11% of the arduino flash is used, and 2K of the 8K ram is used allowing ample space for additions and modifications of the code to suit individual needs.  

The RTC and SD functions are provided by the Adafruit SD datalogging shield:
https://tinyurl.com/yyhj2fle


It uses a "mini-shield" to connect to the Arduino Mega double row expansion connector, and provides a 24 pin (2x12) connector to go to a GPIB connector.  The link to the PCB on OSHPARK is :
https://oshpark.com/shared_projects/mtz1Vv3A

To build the GPIB mini-shield use sixteen 470 ohm 1206 size SMT resistors, a 2x18 straight male header (for the Arduino expansion header), and a 2x12 right angle (or straight) male header for the GPIB connector, and two 1x2 male headers. The 1x2 headers only provide some extra stability for the board.  If you use feed-through stacking headers, you can add more shields on top of this one.

An enclosure is able to be 3D printed.  The enclosure design (STL file type) is included in this project.  There are two separate files, one being the top and one being the bottom of the enclosure.  Some final fine-tuning of the enclosure is likely to be needed to get a perfect fit.  Use a fine file intended for woodworking.  In some places sandpaper will work well too.
