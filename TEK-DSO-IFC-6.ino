
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include "GPIB.h"
#include "Tek_Interface.h"
#include "SVG.h"
#include "Arduino.h"
#include "RTC.h"
#include "EEP.h"
#include <SD.h>
#include <SPI.h>


/*This code will interface to the scope and wait for a SRQ.  It assumes it is ONLY connected to one scope and
 * nothing else.  It does not poll "EVT?"   It simply uses a loop reading the SRQ signal and when it is asserted (LOW)
 * it will read the event and take appropriate action.
 * 
 * There are 5 buttons that are able to be used: event code 450-454 inclusive
 * 1:  Save displayed waveforms to SVG
 * 2:  unused:  Maybe something like FFT
 * 3:  unused:  Maybe to save the front panel setup
 * 4:  unused:  Maybe something like recall the front panel setup
 * 5:  unused:  Use it for a hard reboot of the arduino
 * 
 *   This will not use the switches on the modified SONY-TEKTRONIX unit: the box is passive 
 *   This allows essentially ANY ARDUINO to be used for the GPIB interface as long as it can also use an SD card somehow.
 *   I think that NANO types don't have enough IO pins for full GPIB interface plus SD
 */


/*   Assumes the scope is at address #1! */


// TODO:  have errors printed to the scope screen instead of to the serial port


//extern char data_buffer[];  //defined in Tek_Interface module
extern RTC_PCF8523 rtc;



void setup() {

  
  bool wait_for_scope_boot = false;
  Serial.begin(115200);

 
  Serial.print(F("checking bus"));
  while (!GPIBCheck()) {   //check the GPIB Bus
      //TODO:  send message to 7-segments "SCOP" to indicate no scope present.  OR scroll "NO SCOPE" on the display
      delay(1000);
      Serial.print(F("."));
      wait_for_scope_boot = true;
  }
  Serial.println(F(" Bus OK"));
  

  //if the bus was not on-line when first tested, then the scope is booting up
  // and we need to it for the scope to complete its own initialization 
  if (wait_for_scope_boot)
  {
      Serial.println(F("Scope Boot Delay...30 sec"));
    for (int i=0; i<30;i++) //perform N loops of a 1 second delay
    {
      delay(1000);  //delay 1 second
    }
  }


  EEP_Init();
  RTC_setup();

  DateTime now = rtc.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.println(now.day(), DEC);

    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    
  tek_Setup();

  checkID(); //TODO: use this check to stop further processing?
  introScreen();
  
  tek_Menu(1); 



  //testDataPins();
  //testSPI();
}




//******************************************************************************
void loop() {
  uint16_t event;


  event = 0;

#ifdef USE_SRQ  
//  Serial.println(F("wait for SRQ"));

  while (WaitForSRQ(0, 0xFFFE));  //returns a 1 if timeout occurs.  Keep polling SRQ until it is asserted (LOW): returns before timeout with a value of zero.
  
//    if (SRQ_PIN & SRQ_BIT) Serial.print(F("SRQ NOT ASSERTED (high)"));
//    else Serial.print(F("SRQ IS ASSERTED (low)"));
  event = GetEvent_SRQ();
  if (event) 
  {
    Serial.print(F("Event occurred:"));
    Serial.println(event);
    processEvent(event);
  }
  //no delays: process SRQs as fastr as possible

#else //use polling method
  event = GetEvent();
  if (event) 
  {
    Serial.print(F("Event occurred:"));
    Serial.println(event);
    processEvent(event);
  }
  else delay(1000); //if there is an event read, then don't delay reading the next one in case they are stacked up

#endif
 
}  //end of loop









//*********************************************************
//*
//*  Handle an incoming event
//*     Most will just get ignored!
//*
//*********************************************************
void processEvent(uint16_t evnt)
{
  // main menu [0] = save svg/hide/x/settings/reset cpu
  // settings menu [1] = date/time/x/x/back
  static uint8_t MenuState = 0;

  switch (MenuState)
  {
    case 0: //main menu
     switch (evnt)
     {
       case 652:  //hide display button pressed
         tek_Menu(MenuState, 1);  //show the menu and force it to be displayed
         break;
       case 450:  //Left hand-most button pressed:  save SVG
         processSVG();
         break;
       case 451:   //hide or unhide the menubar
         tek_Menu(MenuState);  //toggle the displayed state of the menu
         break;
       case 452:
         break;
       case 453:
         MenuState = 1;
         tek_Menu(MenuState,1); //show the new menu and force it to be displayed
         break;
       case 454: //maybe use this to hard reset the arduino
         //hideMenu();
         resetCPU();
         break;
      }  //end switch evnt
      break;

    case 1: //settings menu
     switch (evnt)
     {
       case 652:  //hide display button pressed
         tek_Menu(MenuState, 1);  //show the menu and force it to be displayed
         break;
       case 450:  //set date
         //call date setting function
         RTC_Date();
         tek_Menu(MenuState, 1);  //show the menu and force it to be displayed
         break;
       case 451: //set time
         RTC_Time();
         tek_Menu(MenuState, 1);  //show the menu and force it to be displayed
         break;
       case 452: //set file name
         tek_SetFileName();
         tek_Menu(MenuState, 1);  //show the menu and force it to be display
         break;
       case 453:
         break;
       case 454: //move up one menu state
         MenuState--;
         if (MenuState == 0xFF) MenuState = 0; //prevent undershooting menustate accidentally: shouldn't ever be possible
         tek_Menu(MenuState,1); //show the new menu and force it to be displayed
         break;
      }  //end switch evnt
      break;
  }  //end switch MenuState  
}




void resetCPU(void)
{
  //This does a software-only reset without resetting ALL the hardware
  //void(* resetFunc) (void) = 0;//declare reset function at address 0
  //resetFunc(); //call reset 

  //This does a hardware reset like a power-up reset:  All registers ad pins are cleared

  noInterrupts();
  __asm("wdr");
  WDTCSR = (1<<WDCE) | (1<<WDE);
  WDTCSR = (1<<WDE);
  while (1);
}












void testDataPins(void)
{
    Serial.println(F("testing pins: data0, gpib PIN 1"));
    //test the data pins
     while (Serial.available()) Serial.read();   //flush the serial channel
     while (!Serial.available()) {
        GPIBRelease( &DATA0_PORT,&DATA0_DDR, DATA0_BIT);
        delay(10);
        GPIBAssert( &DATA0_PORT,&DATA0_DDR, DATA0_BIT);
        delay(10);
     }

     while (Serial.available()) Serial.read();   //flush the serial channel

     Serial.println(F("testing pins: data1, GPIB PIN 2"));
     while (!Serial.available()) {
        GPIBRelease( &DATA1_PORT,&DATA1_DDR, DATA1_BIT);
        delay(10);
        GPIBAssert( &DATA1_PORT,&DATA1_DDR, DATA1_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel

     Serial.println(F("testing pins: data2, GPIB PIN 3"));
      while (!Serial.available()) {
        GPIBRelease( &DATA2_PORT,&DATA2_DDR, DATA2_BIT);
        delay(10);
        GPIBAssert( &DATA2_PORT,&DATA2_DDR, DATA2_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel

     Serial.println(F("testing pins: data3, GPIB PIN 4"));
     while (!Serial.available()) {
        GPIBRelease( &DATA3_PORT,&DATA3_DDR, DATA3_BIT);
        delay(10);
        GPIBAssert( &DATA3_PORT,&DATA3_DDR, DATA3_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: data4, GPIB PIN 13"));
     while (!Serial.available()) {
        GPIBRelease( &DATA4_PORT,&DATA4_DDR, DATA4_BIT);
        delay(10);
        GPIBAssert( &DATA4_PORT,&DATA4_DDR, DATA4_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel

     Serial.println(F("testing pins: data5, GPIB PIN 14"));
     while (!Serial.available()) {
        GPIBRelease( &DATA5_PORT,&DATA5_DDR, DATA5_BIT);
        delay(10);
        GPIBAssert( &DATA5_PORT,&DATA5_DDR, DATA5_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: data6, GPIB PIN 15"));
     while (!Serial.available()) {
        GPIBRelease( &DATA6_PORT,&DATA6_DDR, DATA6_BIT);
        delay(10);
        GPIBAssert( &DATA6_PORT,&DATA6_DDR, DATA6_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: data7, GPIB PIN 16"));
     while (!Serial.available()) {
        GPIBRelease( &DATA7_PORT,&DATA7_DDR, DATA7_BIT);
        delay(10);
        GPIBAssert( &DATA7_PORT,&DATA7_DDR, DATA7_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: EOI, GPIB PIN 5"));
     while (!Serial.available()) {
        GPIBRelease( &EOI_PORT,&EOI_DDR, EOI_BIT);
        delay(10);
        GPIBAssert( &EOI_PORT,&EOI_DDR, EOI_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: DAV, GPIB PIN 6"));
     while (!Serial.available()) {
        GPIBRelease( &DAV_PORT,&DAV_DDR, DAV_BIT);
        delay(10);
        GPIBAssert( &DAV_PORT,&DAV_DDR, DAV_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: NRFD, GPIB PIN 7"));
     while (!Serial.available()) {
        GPIBRelease( &NRFD_PORT,&NRFD_DDR, NRFD_BIT);
        delay(10);
        GPIBAssert( &NRFD_PORT,&NRFD_DDR, NRFD_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: NDAC, GPIB PIN 8"));
     while (!Serial.available()) {
        GPIBRelease( &NDAC_PORT,&NDAC_DDR, NDAC_BIT);
        delay(10);
        GPIBAssert( &NDAC_PORT,&NDAC_DDR, NDAC_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: IFC, GPIB PIN 9"));
     while (!Serial.available()) {
        GPIBRelease( &IFC_PORT,&IFC_DDR, IFC_BIT);
        delay(10);
        GPIBAssert( &IFC_PORT,&IFC_DDR, IFC_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: SRQ, GPIB PIN 10"));
     while (!Serial.available()) {
        GPIBRelease( &SRQ_PORT,&SRQ_DDR, SRQ_BIT);
        delay(10);
        GPIBAssert( &SRQ_PORT,&SRQ_DDR, SRQ_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: ATN, GPIB PIN 11"));
     while (!Serial.available()) {
        GPIBRelease( &ATN_PORT,&ATN_DDR, ATN_BIT);
        delay(10);
        GPIBAssert( &ATN_PORT,&ATN_DDR, ATN_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("testing pins: REN, GPIB PIN 17"));
     while (!Serial.available()) {
        GPIBRelease( &REN_PORT,&REN_DDR, REN_BIT);
        delay(10);
        GPIBAssert( &REN_PORT,&REN_DDR, REN_BIT);
        delay(10);
     }
     while (Serial.available()) Serial.read();   //flush the serial channel
}



void testSPI(void)
{
     Serial.println(F("Testing SPI MOSI"));
     pinMode(PIN_SPI_MOSI,OUTPUT);
     while (!Serial.available()) {
        digitalWrite(PIN_SPI_MOSI, 1);
        delay(10);
        digitalWrite(PIN_SPI_MOSI, 0);
        delay(10);
     }
     pinMode(PIN_SPI_MOSI,INPUT);
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("Testing SPI MISO"));
     pinMode(PIN_SPI_MISO,OUTPUT);
     while (!Serial.available()) {
        digitalWrite(PIN_SPI_MISO, 1);
        delay(10);
        digitalWrite(PIN_SPI_MISO, 0);
        delay(10);
     }
     pinMode(PIN_SPI_MISO,INPUT);
     while (Serial.available()) Serial.read();   //flush the serial channel


     Serial.println(F("Testing SPI SCK"));
     pinMode(PIN_SPI_SCK,OUTPUT);
     while (!Serial.available()) {
        digitalWrite(PIN_SPI_SCK, 1);
        delay(10);
        digitalWrite(PIN_SPI_SCK, 0);
        delay(10);
     }
     pinMode(PIN_SPI_SCK,INPUT);
     while (Serial.available()) Serial.read();   //flush the serial channel



     Serial.println(F("Testing SPI /CS"));
     pinMode(CS_PIN,OUTPUT);
     while (!Serial.available()) {
        digitalWrite(CS_PIN, 1);
        delay(10);
        digitalWrite(CS_PIN, 0);
        delay(10);
     }
     pinMode(CS_PIN,INPUT);
     while (Serial.available()) Serial.read();   //flush the serial channel
}
