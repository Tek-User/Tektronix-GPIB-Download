#include <avr/pgmspace.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "gpib.h"
#include "Tek_Interface.h"
#include "SVG.h"
#include "Arduino.h"
#include "RTC.h"
#include "EEP.h"
#include <SD.h>
#include <SPI.h>

extern RTC_PCF8523 rtc; //defined in RTC.cpp
extern char data_buffer[];  //defined in Tek_Interface module
//char filename[15];//[] = "TEK000.svg";    //SD card file name

File TEKfile;
uint32_t colors[8] = {0x00AF00,0x0000FF, 0xFF00FF, 0x00A0A0, 0xA0A000, 0x005F7F, 0xFF6600, 0xFF0066};
//uint32_t colors[8] = {0x2ECC40, 0x7FDBFF, 0x39CCCC, 0x0074D9, 0x01FF70, 0x001F3F, 0x3D9970, 0xFDC00};  //green, blue, aqua, teal, lime, navy, olive, yellow




//*****************************************************************
//*
//*      dateTime callback
//*        used by the SDFat functions to retrieve 
//*        the current date and time so set the file creation info
//*
//******************************************************************
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}




//*************************************************************************************
//*
//*    Start the process of saving the displayed waveforms to an SVG graphics file
//*
//*************************************************************************************
void processSVG(void)
{
   uint8_t status;
   
  //Serial.println(F("starting to create file"));
  status = SD_Setup(); //Opens "filename" on the SD card root directory.  TEKfile is the handle
  if (!status){
      //TODO:  send message to 7-segments "REAd" to indicate it is reading the scope and transferring data
      startSVG();
      print_graticule();  //skip to just save some time and space for testing of the GPIB printing aspects
      print_channels();
      endSVG();
 Serial.println(F("Closing file"));
      TEKfile.close();
      tek_Message((char) 0);  //clear the message line
  }

  else 
  {
      if (status == 1){
        Serial.println(F("NO CARD!"));
        strcpy_P(data_buffer, PSTR("SD CARD FAILED OR NOT PRESENT"));
      }
  
      else if (status == 2) {
        Serial.println(F("UNABLE TO OPEN FILE ON SD CARD"));   
        strcpy_P(data_buffer, PSTR("UNABLE TO OPEN FILE"));
      }
      else {
        Serial.print(F("OTHER SD ERROR:"));
        Serial.println(status);
        strcpy_P(data_buffer, PSTR("OTHER SD ERROR"));
      }
      tek_RingBell();
      tek_Message(data_buffer);
      delay(4000);
      tek_Message((char) 0);  //clear the line     
  }
  Serial.println(F("Done!"));  
}








//***************************************************************************************
//*
//*       print the displayed channel info and traces
//*
//*           Receives: VOID
//*           Returns: VOID
//*
//***************************************************************************************
void print_channels(void)
{
     uint8_t textline = 0;    //textline is used to control position and color of the channel
     uint8_t displayed[NUMCHANNELS];
     
    //go thru the channels and read them
    displayedChannels(displayed); 
    
    for (int channel = 0; channel < NUMCHANNELS; channel++)
    {
        if (displayed[channel] == 1)
        {
             ReadWfmPre(channel);  //data is returned in global data_buffer
             displayText(textline);
             ReadCurve(channel);
             displayCurve(textline);
             textline++;
      }
   }
}





//***************************************************************************************
//*
//*       Display the trace from the scope
//*
//*           Receives: the line number to control color.  Data is in global data_buffer
//*           Returns: VOID
//*
//***************************************************************************************
void displayCurve(uint8_t textline)
{
    TEKfile.println();
    //Serial.println (F("<g transform = \"translate(2,2)\" stroke=\"#00AF00\" fill=\"none\" stroke-width=\"1\">"));  //start a group
    TEKfile.print(F("<g transform = \"translate(2,2)\" stroke=\"#"));
    if (colors[textline] < 0x100000) TEKfile.print(F("0"));  //pad leading zeros if necessary
    if (colors[textline] < 0x010000) TEKfile.print(F("0"));  //pad leading zeros if necessary
    if (colors[textline] < 0x001000) TEKfile.print(F("0"));  //pad leading zeros if necessary
    if (colors[textline] < 0x000100) TEKfile.print(F("0"));  //pad leading zeros if necessary
    TEKfile.print(colors[textline],HEX);
    TEKfile.print(F("\" fill=\"none\" stroke-width=\"1\">"));  //start a group


    TEKfile.print(F("<polyline points=\""));
                       
    for (uint16_t index = 0; index < NUMPOINTS; index++)
    {
        TEKfile.print(index);
        TEKfile.print(F(","));
        //TEKfile.print(2*(255-data_buffer[index]));
        TEKfile.print(2* (uint16_t)(255-(uint8_t)data_buffer[index]));
        TEKfile.print(F(" "));
    }

    TEKfile.println(F("\"/>"));
    TEKfile.println (F("</g>")); //finish group
}








//***************************************************************************************
//*
//*       Display a text line below the tracings and graticule
//*
//*           Receives: the line number for the text and to control color.  Data is in global data_buffer
//*           Returns: VOID
//*
//***************************************************************************************
void displayText(uint8_t textline)
{
//TODO:  Stroke and Fill should be variables based on text line number
    TEKfile.println();
    //Serial.println (F("<g transform = \"translate(2,2)\" stroke=\"#00AF00\" fill=\"#00AF00\" stroke-width=\"1\" font-size=\"20px\"  style=\"font-family: Arial, Tahoma, Verdana, Sans-Serif; font-weight: normal; font-style: normal\">"));  //start a group
    TEKfile.print(F("<g transform = \"translate(2,2)\" stroke=\"#"));
    if (colors[textline] < 0x100000) TEKfile.print(F("0"));  //pad leading zeros if necessary
    if (colors[textline] < 0x010000) TEKfile.print(F("0"));  //pad leading zeros if necessary
    if (colors[textline] < 0x001000) TEKfile.print(F("0"));  //pad leading zeros if necessary
    if (colors[textline] < 0x000100) TEKfile.print(F("0"));  //pad leading zeros if necessary
    TEKfile.print(colors[textline],HEX);
    TEKfile.print(F("\" fill=\"#"));
    if (colors[textline] < 0x100000) TEKfile.print(F("0"));  //pad leading zeros if necessary
    if (colors[textline] < 0x010000) TEKfile.print(F("0"));  //pad leading zeros if necessary
    if (colors[textline] < 0x001000) TEKfile.print(F("0"));  //pad leading zeros if necessary
    if (colors[textline] < 0x000100) TEKfile.print(F("0"));  //pad leading zeros if necessary
    TEKfile.print(colors[textline],HEX);
    TEKfile.println(F("\" stroke-width=\"1\" font-size=\"20px\"  style=\"font-family: Arial, Tahoma, Verdana, Sans-Serif; font-weight: normal; font-style: normal\">"));  //start a group
    TEKfile.print (F("<text x=\"20\" y=\""));
    TEKfile.print (535+(textline*25));
    TEKfile.print(F("\">"));
    TEKfile.print(data_buffer);
    TEKfile.println(F("</text>")); 
    TEKfile.println (F("</g>")); //finish group
}





//***************************************************************************************
//*
//*       print the start of the SVG file
//*
//*           Receives: VOID
//*           Returns: VOID
//*
//***************************************************************************************
void startSVG(void)
{
    // setup the SVG vector graphics file
    TEKfile.println (F("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"));
    // this is the entire page size, be sure to make it just big enough for all the stuff or else it is likely to be cropped!
    TEKfile.println (F("<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.2\" baseProfile=\"full\" width=\"1027px\" height=\"715px\">"));
    TEKfile.println ();
}




//***************************************************************************************
//*
//*       print the end of the SVG file
//*
//*           Receives: VOID
//*           Returns: VOID
//*
//***************************************************************************************
void endSVG(void)
{
     //End the SVG vector graphics file
     TEKfile.println();
     TEKfile.println (F("</svg>"));   //signifies the end of the SVG graphics
}






//***************************************************************************************
//*
//*       print the graticule
//*
//*           Receives: VOID
//*           Returns: VOID
//*
//***************************************************************************************
void print_graticule(void) {
  #define max_x 1024
  #define max_y 511
  uint16_t x_offset = 12;
  uint16_t y_offset = 5;
  uint16_t x_pos = 0;
  uint16_t y_pos = 0;
  char x_str[10];
  char str_2[10];
  char y_str[10];
  
 
  
  //move the origin to 1,1 (absolute) so the entire bounding box will show for sure
  TEKfile.println (F("<g transform = \"translate(1,1)\" stroke=\"#FF0000\" stroke-width=\"1\">"));
  TEKfile.println (F("<rect x=\"0\" y=\"0\" width=\"1025\" height=\"512\" fill=\"white\" stroke-width=\"2\" stroke=\"#FF0000\" />"));
  TEKfile.println (F("</g>"));
  TEKfile.println ();
  //TODO: VERIFY THE BOUNDING BOX IS JUST OUTSIDE THE DRAWING SPACE FOR THE GRATICULES AND DATA
  

   //and draw the graticules: 4 wide, 2 high:  50 samples per DIV, 5 div per major line, 1000 total.  Add an x_offset if desired.
   //Draw the vertical lines
   //Always position the origin inside the bounding box... for all further screen-related drawing including the actual data

   //Draw the minor vertical lines
   //interval = 50 points per div and avoid first and last lines
  TEKfile.println (F("<g transform = \"translate(2,2)\" stroke=\"#FFBFBF\" stroke-width=\"1\">"));
  for (x_pos = 50; x_pos < max_x-50; x_pos+= 50)
  {
     TEKfile.print (F("<line x1=\""));
     itoa(x_pos+x_offset, x_str,10);
     TEKfile.print (x_str);
     TEKfile.print (F("\" y1=\"0\" x2=\""));
     TEKfile.print (x_str);
     TEKfile.println(F("\" y2=\"510\"/>"));

     //draw the minor ticks
     itoa(x_pos+x_offset-1, x_str,10);
     itoa(x_pos+x_offset+1, str_2,10);
     for (y_pos = 0; y_pos < max_y-y_offset; y_pos += 10)
     {
          itoa(y_pos+y_offset, y_str,10);
          TEKfile.print (F("<line x1=\""));
          TEKfile.print (x_str);
          TEKfile.print (F("\" y1=\""));
          TEKfile.print (y_str);
          TEKfile.print (F("\" x2=\""));
          TEKfile.print (str_2);
          TEKfile.print(F("\" y2=\""));
          TEKfile.print (y_str);
          TEKfile.println(F("\"/>")); 
     }
   }

  TEKfile.println (F("</g>"));
  TEKfile.println ();

  //draw the minor horizontal lines and ticks here
  //interval = 25 points per div and avoid first and last lines
  //multiply y coord by 2 for plotting
  TEKfile.println (F("<g transform = \"translate(2,2)\" stroke=\"#FFBFBF\" stroke-width=\"1\">"));
  for (y_pos = 50; y_pos < max_y-50; y_pos+= 50)
  {
     TEKfile.print (F("<line x1=\"0\" y1=\""));
     itoa(y_pos+y_offset, y_str,10);
     TEKfile.print (y_str);
     TEKfile.print (F("\" x2=\"1023\" y2=\""));
     TEKfile.print (y_str);
     TEKfile.println(F("\"/>"));

     //draw the minor ticks  
     itoa(y_pos+y_offset-1, y_str,10);
     itoa(y_pos+y_offset+1, str_2,10);
     for (x_pos = 0; x_pos < max_x-x_offset; x_pos += 10)
     {
          itoa(x_pos+x_offset, x_str,10);
          TEKfile.print (F("<line x1=\""));
          TEKfile.print (x_str);
          TEKfile.print (F("\" y1=\""));
          TEKfile.print (y_str);
          TEKfile.print (F("\" x2=\""));
          TEKfile.print (x_str);
          TEKfile.print(F("\" y2=\""));
          TEKfile.print (str_2);
          TEKfile.println(F("\"/>")); 
     }
     
   }

  TEKfile.println (F("</g>"));
  TEKfile.println ();

  

 
   //Draw the major vertical lines
   //interval = 5div * 50 points per div = 250
  TEKfile.println (F("<g transform = \"translate(2,2)\" stroke=\"#FF5F5F\" stroke-width=\"1\">"));
  for (x_pos = 0; x_pos < max_x; x_pos+= 250)
  {
     TEKfile.print (F("<line x1=\""));
     itoa(x_pos+x_offset, x_str,10);
     TEKfile.print (x_str);
     TEKfile.print (F("\" y1=\"0\" x2=\""));
     TEKfile.print (x_str);
     TEKfile.println(F("\" y2=\"510\"/>"));
     //draw the major ticks here
     itoa(x_pos+x_offset-3, x_str,10);
     itoa(x_pos+x_offset+3, str_2,10);
     //if(x_pos>0 && x_pos < max_x-250)
     {
        for (y_pos = 0; y_pos < max_y-y_offset; y_pos += 10)
        {
             itoa(y_pos+y_offset, y_str,10);
             TEKfile.print (F("<line x1=\""));
             TEKfile.print (x_str);
             TEKfile.print (F("\" y1=\""));
             TEKfile.print (y_str);
             TEKfile.print (F("\" x2=\""));
             TEKfile.print (str_2);
             TEKfile.print(F("\" y2=\""));
             TEKfile.print (y_str);
             TEKfile.println(F("\"/>")); 
        }
     }
  }
  TEKfile.println (F("</g>"));
  TEKfile.println ();

  //Draw the major horizontal lines and ticks here
  TEKfile.println (F("<g transform = \"translate(2,2)\" stroke=\"#FF5F5F\" stroke-width=\"1\">"));
  for (y_pos = 0; y_pos < max_y; y_pos+= 250)
  {
     TEKfile.print (F("<line x1=\"0\" y1=\""));
     itoa(y_pos+y_offset, y_str,10);
     TEKfile.print (y_str);
     TEKfile.print (F("\" x2=\"1023\" y2=\""));
     TEKfile.print (y_str);
     TEKfile.println(F("\"/>"));

     //draw the Major ticks here
     itoa(y_pos+y_offset-3, y_str,10);
     itoa(y_pos+y_offset+3, str_2,10);
     for (x_pos = 0; x_pos < max_x-x_offset; x_pos += 10)
     {
          itoa(x_pos+x_offset, x_str,10);
          TEKfile.print (F("<line x1=\""));
          TEKfile.print (x_str);
          TEKfile.print (F("\" y1=\""));
          TEKfile.print (y_str);
          TEKfile.print (F("\" x2=\""));
          TEKfile.print (x_str);
          TEKfile.print(F("\" y2=\""));
          TEKfile.print (str_2);
          TEKfile.println(F("\"/>")); 
     }
   }

  TEKfile.println (F("</g>"));
  TEKfile.println ();



}


//***********************************************************
//*
//*
//*
//***********************************************************
 uint8_t SD_Setup (void)
{
    uint8_t pos; //position of the first number in the serialization
    char filename[15];//[] = "TEK000.svg";    //SD card file name
    
    //setup the SD card and open a file.  return errors
    //0 = OK
    //1 = no card
    //2 = can't open a valid file
    pinMode(CS_PIN, OUTPUT);

    //IF the SD.begin() fails to work the SECOND time it is run.
    //you need to modify the SD.cpp code in the SD library:
    //add "root.close()" just before the following line in the code:
    //  return card.init(SPI_HALF_SPEED, csPin) &&
    //     volume.init(card) &&
    //     root.openRoot(volume);
     
   
    //Serial.println(F("SD_Begin!"));
    if (!SD.begin(CS_PIN)) {
        Serial.println(F("Card failed, or not present"));
        // don't do anything more:
        return 1;
    }
   

    
//Serial.print(F("First_time set to"));
//Serial.println(first_time);
    
    //Serial.println(F("card initialized."));
  
    // create a new file
    EEP_ReadNameStub(filename);   //the name stub is 1-5 characters
//Serial.print(F("read filename from EEP >"));
//Serial.print(filename);
//Serial.println(F("<"));
    pos = strlen(filename);   // index is the position of the first digit in the serialization number after the 1-5 characters
//Serial.println(pos); 
    strcat_P(filename, PSTR("000.svg")); //now the name is Xxxxx000.svg
//Serial.println(filename);
    //strcpy(filename, "ABC000.svg");

    for (uint8_t i = 0; i < 1000; i++) {
        filename[pos] = i/100 + '0';      //modulo method as fast or faster than anything else
        filename[pos+1] = (i%100)/10 + '0';
        filename[pos+2] = (i%10) + '0';
//Serial.println(filename);        
        if (! SD.exists(filename)) {
          // only open a new file if it doesn't exist
          SdFile::dateTimeCallback(dateTime);
          TEKfile = SD.open(filename, FILE_WRITE); 
          strcpy_P(data_buffer, PSTR("WRITING TO FILE: "));
          strcat(data_buffer,filename);
          tek_Message(data_buffer);
          break;  // leave the loop!
        }
    }
  
  if (! TEKfile) {
    Serial.println(F("couldn't create file"));
    Serial.println(filename);
    return 2;
  }
  
  Serial.print(F("Logging to: "));
  Serial.println(filename);
  //TEKfile.close();
  return 0;

}
