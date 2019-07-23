# Tektronix-GPIB-Download
Software (and hardware recommendation) to allow copying your oscilloscope display directly to a flash drive

This purpose of this project is to create an inexpensive, and simple-to-build device which attaches to Tektronix 2430, 2430A, 2432, and 2440 digital storage oscilloscopes to allow the user to download the displayed trace(s) to a flash drive without the need for a computer to be attached to the oscilloscope.  No hardware beyond the device and a GPIB cable are required.  The downloaded file on the flash drive is to be a vector graphics file type which can be viewed and printed using readily available free software.  

An Arduino (or clone) Mega2560 is required as the smaller devcices do not have enough I/O pins to do the job properly.

The program interfaces to the oscilloscope front panel user-interface buttons.  It will save the displayed traces to a SVG file along with the volts/div and sec/div settings.  All 1024 data points are stored in the vector graphics file.

Control menus allow storing the trace(s), setting date/time/filename.

About 11% of the arduino flash is used, and 2K of the 8K ram is used allowing ample space for additions and modifications of the code to suit individual needs.   
