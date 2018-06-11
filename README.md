# Tektronix-GPIB-Download
Software (and hardware recommendation) to allow copying your oscilloscope display directly to a flash drive

This purpose of this project is to create an inexpensive, and simple-to-build device which attaches to Tektronix 2430, 2430A, 2432, and 2440 digital storage oscilloscopes to allow the user to download the displayed trace(s) to a flash drive without the need for a computer to be attached to the oscilloscope.  No hardware beyond the device and a GPIB cable are required.  The downloaded file on the flash drive is to be a vector graphics file type which can be viewed and printed using readily available free software.  

The device consists of an Arduino microcontroller board (E.G. the Arduino Nano clone), and a CH376-based board to write to a flash drive.

The project will also include files for fabrication of a PCB on which to mount the Arduino and a GPIB interface (Centronix) socket, though it would be feasible to hack a GPIB cable and individually solder the wires to the PCB.

It is likely to be possible to interface to other GPIB devices with the proper modifications to the Arduino code by someone acquainted with the GPIB instructions required by the device.  Additionally, it is likely to be possible to use a serial interface to devices instead of GPIB with proper modification of the Arduino code.

The file saved to the flash drive is currently SVG format whch is supported by most internet "explorer"" type apps such as Firefox, Chrome, etc.  Also SVG files can be converted to PDF file type using free on-line services (E.G Zamzar) or proprietary programs (E.G. Adobe Illustrator).

UPDATE June 11, 2018
I will soon be uploading a functional project.  I had a very hard time getting the CH376S working.  It is very poorly documented.  It appears impossible to conveniently write large batches of data using the asynch serial method.  I would have tried the SPI interface but the CH376 board I have does not suopport SPI despite the chip itself having that ability.

Due to this trouble, I decided to go with SD/CF card.  The Arduino platform offers built-in SPI and SD capability with FAT handling.  I have this up and working after just an hour or so of effort.  Over a week of struggling with the CH376 had the USB flash host barely functional with major limitations.

I am now working on integrating the GPIB functions to the project which already has the functioning SD card interface.
