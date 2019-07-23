#include "gpib.h"
#include "Arduino.h"
//#include "Tek_Interface.h"




//*******************************************************************************
//*
//*   Test for Instrument on the GPIB Bus
//*
//*       returns TRUE if a device is on the bus, FALSE if nothing is there
//*
//*******************************************************************************
int GPIBCheck(void)
{
    uint8_t DDR_set, PORT_set, retval;
    //when devices are not addressed, they let the bus float high
    //assert ATN and that places devices into listen mode
    //in listen mode a device will pull NDAC low
    //
    DDR_set = NDAC_DDR & NDAC_BIT;      //record if the DDR bit is set so it can be restored
    PORT_set = NDAC_PORT & NDAC_BIT;    //record if the PORT bit is set so it can be restored
    NDAC_DDR &= ~NDAC_BIT;  //make NDAC input
    //NDAC_PORT &= ~NDAC_BIT; //make tristate   TODO: MAYBE make it weakly pulled up?
    NDAC_PORT |= NDAC_BIT;  //enable weak pull-up
    
    GPIBInterfaceClear();   //reset the interface if it is in the middle of something
    GPIBAssert(&ATN_PORT, &ATN_DDR, ATN_BIT);   //assert ATN  
    delay(1);  //delay a moment just to let things get processed: not absolutely required

    if (NDAC_PIN & NDAC_BIT) retval = 0;    //if the pin is high (not asserted then no device is on the bus: return false
    else retval = 1;                        //if the pin is low, then a device is on the bus: return true

    if (DDR_set) NDAC_DDR |= NDAC_BIT; 
    else NDAC_DDR &= ~NDAC_BIT;

    if (PORT_set) NDAC_PORT |= NDAC_BIT; 
    else NDAC_PORT &= ~NDAC_BIT;

    GPIBInterfaceClear();   //reset the interface
    return retval; 
}




//******************************************************************************
//******************************************************************************
void GPIBInit(void)
{  
  //lets try setting it up to be a listener by default
  //NDAC and NRFD need to be actively pulled
  GPIBAssert(&NDAC_PORT,&NDAC_DDR, NDAC_BIT);  // Active data not accepted: pulled low
  GPIBRelease(&NRFD_PORT, &NRFD_DDR, NRFD_BIT); // Active ready for data: pulled high

  GPIBClear(&DAV_PORT, &DAV_DDR, DAV_BIT);  //passively let the bus float, maybe with weak pull-ups
  GPIBClear(&EOI_PORT, &EOI_DDR, EOI_BIT); 
  GPIBClear(&ATN_PORT, &ATN_DDR, ATN_BIT);
  
  GPIBClear(&SRQ_PORT, &SRQ_DDR, SRQ_BIT);    //weak pull up on the SRQ, so other devices can signal
  GPIBRelease(&IFC_PORT, &IFC_DDR, IFC_BIT);  //actively pull the pins because only controller uses these
  GPIBRelease(&REN_PORT, &REN_DDR, REN_BIT);
  
  GPIBClearData();  //set data bits to input-tristate
}




//********************************************************************************************************
//*
//*  High level function to write a block of data
//*  Receives a pointer to a block of bytes, the length of the block, and whether to use EOI signalling
//*  Returns the number of bytes sent or zero if there is an error
//*
//********************************************************************************************************

uint16_t GPIBWriteData(char* data, uint16_t count, bool useEOI)
{
  //writes an entire array of data to the GPIB bus
  uint16_t counter;
  uint8_t error = 0;
  bool transEOI = false;
  uint16_t last = count-1;
  uint16_t retval;
  
//Serial.print(F(" Write Data "));
//GPIBInitTalk();
  
  for (counter = 0; counter < count; counter++)
  {
    if ((counter == last) && useEOI) transEOI= true;
    error = GPIBWriteByte(data[counter], transEOI);
  }
  
  if (error) retval = 0;
  else retval = count;
//Serial.print(F(" EXIT Write Data "));
  return retval;  
}





//***********************************************
//* 
//*  Send a null terminated string to the bus
//*
//***********************************************
uint16_t GPIBWriteString (char* data, bool useEOI)
{
  uint16_t counter;
  uint8_t error = 0;
  bool transEOI = false;
  uint16_t retval;

//GPIBInitTalk();
  while(!transEOI)
  {
    if (data[counter+1] == 0) transEOI = true;    //if the next character is the null terminator, then this is the last 
    error = GPIBWriteByte(data[counter], transEOI); 
    counter++;
  }
  
  if (error) retval = 0;
  else retval = counter;
  return retval; 
  
}




//*********************************************************************
//*
//* A wrapper of writedata to/ assert/release ATN to form a command
//*
//*********************************************************************

void GPIBWriteCmd(char* data, uint16_t _length, bool useEOI)
{
//Serial.print(F(" Write Cmd "));
  GPIBAssert(&ATN_PORT, &ATN_DDR, ATN_BIT);
  GPIBWriteData(data, _length, useEOI);
  GPIBRelease(&ATN_PORT, &ATN_DDR, ATN_BIT);
}


















//***********************************************
//***********************************************
void GPIBRelease(volatile uint8_t *gpibPORT, volatile uint8_t *gpibDDR, uint8_t gpibBIT)
{
  //make the line OUTPUT-HIGH
  
  //set DDR to output (Set the bit)
  *gpibDDR |= gpibBIT;
  
  //Set the PORT to HIGH (set the bit)
  *gpibPORT |= gpibBIT;


  
}




//***********************************************
//***********************************************
void GPIBAssert(volatile uint8_t *gpibPORT, volatile uint8_t *gpibDDR, uint8_t gpibBIT)
{
  //make the line OUTPUT-LOW

  //Set DDR to OUTPUT (Set the bit)
  *gpibDDR |= gpibBIT;
  
  //Set PORT to LOW (clear the bit)
  *gpibPORT &= ~gpibBIT;
  //PORTA &= ~gpibBIT;
  
/*if (*gpibPORT == NDAC_PORT)
{
    Serial.print (F("Asserting NDAC "));
    Serial.print((uint8_t)gpibPORT);
    Serial.print(F(" "));
    Serial.print((uint8_t)gpibDDR);
    Serial.print(F(" "));
    Serial.println(gpibBIT,BIN);
    
}*/

}




//***********************************************
//***********************************************
void GPIBClear(volatile uint8_t *gpibPORT, volatile uint8_t *gpibDDR, uint8_t gpibBIT)
{
  //make the line INPUT-PULLUP 

  //Set DDR to INPUT (Clear the bit)
  *gpibDDR &= ~gpibBIT;
  
  //Set PORT to HIGH (set the bit)
  *gpibPORT |= gpibBIT;  
}



//***********************************************
//***********************************************
void GPIBClearData(void)
{
 //Do it the hard slow way but universally acceptable: set individual pins.
 //do it in this order:  When turning a pin OUTPUT-HIGH/LOW, first set the value, then set the direction
 //                      When turning a pin INPUT-HIGH/LOW, first set the direction, then set the value

 //Clear DDR AND PORT for each data pin (input-tristate)
  DATA0_DDR  &= ~DATA0_BIT;
  DATA0_PORT &= ~DATA0_BIT; 

  DATA1_DDR  &= ~DATA1_BIT;
  DATA1_PORT &= ~DATA1_BIT; 

  DATA2_DDR  &= ~DATA2_BIT;
  DATA2_PORT &= ~DATA2_BIT; 

  DATA3_DDR  &= ~DATA3_BIT;
  DATA3_PORT &= ~DATA3_BIT;

  DATA4_DDR  &= ~DATA4_BIT;
  DATA4_PORT &= ~DATA4_BIT;
  
  DATA5_DDR  &= ~DATA5_BIT;
  DATA5_PORT &= ~DATA5_BIT;

  DATA6_DDR  &= ~DATA6_BIT;
  DATA6_PORT &= ~DATA6_BIT;

  DATA7_DDR  &= ~DATA7_BIT;
  DATA7_PORT &= ~DATA7_BIT;
}







//***********************************************
//***********************************************
void GPIBClearData_Talk(void)
{
 //Do it the hard slow way but universally acceptable: set individual pins.
 //do it in this order:  When turning a pin OUTPUT-HIGH/LOW, first set the value, then set the direction
 //                      When turning a pin INPUT-HIGH/LOW, first set the direction, then set the value

 //MAKE DDR OUTPUT AND SET THE BITS HIGH
 
  DATA0_PORT |= DATA0_BIT; 
  DATA0_DDR  |= DATA0_BIT;

  DATA1_PORT |= DATA1_BIT; 
  DATA1_DDR  |= DATA1_BIT;

  DATA2_PORT |= DATA2_BIT; 
  DATA2_DDR  |= DATA2_BIT; 

  DATA3_PORT |= DATA3_BIT; 
  DATA3_DDR  |= DATA3_BIT;

  DATA4_PORT |= DATA4_BIT; 
  DATA4_DDR  |= DATA4_BIT;
  
  DATA5_PORT |= DATA5_BIT; 
  DATA5_DDR  |= DATA5_BIT;

  DATA6_PORT |= DATA6_BIT; 
  DATA6_DDR  |= DATA6_BIT;

  DATA7_PORT |= DATA7_BIT; 
  DATA7_DDR  |= DATA7_BIT;
}



//***********************************************
//***********************************************
void GPIBInitListen(void)
{
  GPIBAssert(&NDAC_PORT, &NDAC_DDR, NDAC_BIT);   // Active data not accepted
  GPIBRelease(&NRFD_PORT, &NRFD_DDR, NRFD_BIT);  // active ready for data
  GPIBClearData();  //set data bits to input-tristate to listen 
  GPIBClear(&DAV_PORT, &DAV_DDR, DAV_BIT); // listener does not control 
  GPIBClear(&EOI_PORT, &EOI_DDR, EOI_BIT); // listener does not control 
  GPIBClear(&ATN_PORT, &ATN_DDR, ATN_BIT); // listener does not control 
}



//***********************************************
//***********************************************
void GPIBInitTalk()
{
  GPIBRelease(&DAV_PORT, &DAV_DDR, DAV_BIT); // active pull up
  GPIBRelease(&EOI_PORT, &EOI_DDR, EOI_BIT); // active pull up
  GPIBRelease(&ATN_PORT, &ATN_DDR, ATN_BIT); // active pull up
  GPIBClearData_Talk();  //set data bits to output-high for talking
  GPIBClear(&NDAC_PORT, &NDAC_DDR, NDAC_BIT);  // passive
  GPIBClear(&NRFD_PORT, &NRFD_DDR, NRFD_BIT);  // Passive
}






//***********************************************
//***********************************************
void  GPIBInterfaceClear(void)
{
  //need to hold IFC asserted for at least 150uS - do it for 300us just to be sure
  GPIBAssert(&IFC_PORT, &IFC_DDR, IFC_BIT);
  delayMicroseconds(300);
  GPIBRelease(&IFC_PORT, &IFC_DDR, IFC_BIT);
}




//***********************************************
//***********************************************
void GPIBRemoteEnable(bool state)
{
  if(state) GPIBAssert(&REN_PORT, &REN_DDR, REN_BIT);
  else GPIBRelease(&REN_PORT, &REN_DDR, REN_BIT);

}




//***********************************************
//***********************************************
uint8_t WaitForDAV(uint8_t state, uint16_t timeLimit)
{
  //wait for DAV to change to a specificed state with a timeout
  uint16_t timeOut = 0;
  uint8_t retval = 0;
  uint8_t reading = 0;
  
  if (DAV_PIN & DAV_BIT) reading = 1;  

  while ((reading != state) && (timeOut < timeLimit))
  {
    delay(1);
    if (DAV_PIN & DAV_BIT) reading = 1;
    else reading = 0;
    timeOut += 1;
  }
  
  if (timeOut == timeLimit) retval = 1; //ONE indicates timeout error
  return retval;
}






//***********************************************
//***********************************************
uint8_t WaitForNRFD(uint8_t state, uint16_t timeLimit)
{
  //wait for NRFD to change to a specificed state with a timeout
  uint16_t timeOut = 0;
  uint8_t retval = 0;
  uint8_t reading = 0;
  
  if (NRFD_PIN & NRFD_BIT) reading = 1;  

  while ((reading != state) && (timeOut < timeLimit))
  {
    delay(1);
    if (NRFD_PIN & NRFD_BIT) reading = 1;
    else reading = 0;
    timeOut += 1;
  }
  
  if (timeOut == timeLimit) retval = 1; //ONE indicates timeout error
  return retval;
}





//***********************************************
//***********************************************
uint8_t WaitForNDAC(uint8_t state, uint16_t timeLimit)
{
  //wait for NDAC to change to a specificed state with a timeout
  uint16_t timeOut = 0;
  uint8_t retval = 0;
 uint8_t reading = 0;
  
  if (NDAC_PIN & NDAC_BIT) reading = 1;  

  while ((reading != state) && (timeOut < timeLimit))
  {
    delay(1);
    if (NDAC_PIN & NDAC_BIT) reading = 1;
    else reading = 0;
    timeOut += 1;
  }
  
  if (timeOut == timeLimit) retval = 1; //ONE indicates timeout error
  return retval;
}






//****************************************************************************
//*
//*  WaitForNNN returns a zero indicating success and 1 indicating timeout
//* 
//****************************************************************************
uint8_t WaitForSRQ(uint8_t state, uint16_t timeLimit)
{
  //wait for SRQ to change to a specificed state with a timeout
  uint16_t timeOut = 0;
  uint8_t retval = 0;
  uint8_t reading = 0;

//  Serial.print(F("SRQ await state of:"));
//  Serial.println(state);
  if (SRQ_PIN & SRQ_BIT) reading = 1;  

  do
  {
    delay(1);
    if (SRQ_PIN & SRQ_BIT) reading = 1;
    else reading = 0;
    timeOut += 1;
  } while ((reading != state) && (timeOut < timeLimit));
  
  if (timeOut == timeLimit) retval = 1; //ONE indicates timeout error
  return retval;
}














//***********************************************
//***********************************************

uint16_t GPIBGetData(char* buffer, uint16_t len, uint16_t timeOut) 
{    
    //Listener function
  //reads data off the bus and transmits it immediately by serial
  //returns number of bytes received
  
  
  /*
          Sequence of events:
          Listener: Assert NDAC, Release NRFD (ready for data)
          Talker:   Asserts DAV after placing new data
          Listener: Asserts NRFD (not ready for new data:  tells talker to hold the current data)
          Listener: Read data
          Listener: Release NDAC (data is accepted)
          Talker:   Release DAV
          Listener: Assert NDAC, Release NRFD
  */
  
  uint16_t retval = 0;
  
  bool last  = false;   

  //GPIBInitListen();
  
//Should ALREADY be in "listen mode so the bus is correct for listening but set the handshaking anyway
  
  GPIBAssert(&NDAC_PORT, &NDAC_DDR, NDAC_BIT);  //indicate data is no longer accepted
  GPIBRelease(&NRFD_PORT, &NRFD_DDR, NRFD_BIT); //indicate ready for new data
  
  while (last == false)
  {
    if (WaitForDAV(_LOW, timeOut))  //wait for Data to be AVailable (LOW), with a timeout
    {
        retval = 0;
//Serial.println(F("DAV never asserted!"));
        break;
    }
   
    GPIBAssert(&NRFD_PORT, &NRFD_DDR, NRFD_BIT);   //indicate Not Ready For new Data
    if ((EOI_PIN & EOI_BIT) == 0) last = true;   //if the bit is low (asserted) by the client, then this is the last byte of the transmission
    
   //read the data 
    buffer[retval] = GPIBReadByte();
//Serial.write(buffer[retval]);   //debug print the data
    retval++;
    if(retval >= len) break;
    
    
    GPIBRelease(&NDAC_PORT, &NDAC_DDR, NDAC_BIT); //release NDAC to indicate that data is accepted
//try releasing NRFD and say we are ready for new data
GPIBRelease(&NRFD_PORT, &NRFD_DDR, NRFD_BIT); //indicate ready for new data

    if (WaitForDAV(_HIGH, timeOut))   //wait for data availability to be de-asserted (high) by the client, with a timeout
    {
        retval = 0;
//Serial.println(F("DAV never released!"));
//Serial.print(F("DAV: "));
//  if (DAV_PIN & DAV_BIT) Serial.print(F("Signal is RELEASED (high) "));
//  else Serial.print(F("Signal is ASSERTED (low), "));
//  if (DAV_PORT & DAV_BIT) Serial.print(F("PORT is RELEASED (high) "));
//  else Serial.print(F("PORT is ASSERTED (low), "));
//  if(DAV_DDR & DAV_BIT) Serial.println(F("DDR: OUTPUT "));
//  else Serial.println(F("DDR: INPUT "));

        break;
    }

    GPIBAssert(&NDAC_PORT, &NDAC_DDR, NDAC_BIT);  //indicate data is no longer accepted
    GPIBRelease(&NRFD_PORT, &NRFD_DDR, NRFD_BIT); //indicate ready for new data
  }  //end while
  
  //Serial.print(_CR);
  //Serial.print(_LF);
  //GPIBAssert(NDAC_PORT, NDAC_DDR, NDAC_BIT);
  //GPIBAssert(NRFD_PORT, NRFD_DDR, NRFD_BIT);
  GPIBInitListen();
  //finish with both NDAC and NRFD asserted to show 
  //a listener is on the bus, but is not ready for data
  return retval;
  
}







//***********************************************
//***********************************************
uint8_t GPIBReadByte(void)
{
  
  uint8_t a =0;


//do it the hard slow but universally functional way.  
//low is a "1" and high is a "0" bit

//Turn on set bits (set low on the bus)

  if (!(DATA7_PIN & DATA7_BIT)) a |= 0x80; 
  if (!(DATA6_PIN & DATA6_BIT)) a |= 0x40;
  if (!(DATA5_PIN & DATA5_BIT)) a |= 0x20;
  if (!(DATA4_PIN & DATA4_BIT)) a |= 0x10;
  if (!(DATA3_PIN & DATA3_BIT)) a |= 0x08;
  if (!(DATA2_PIN & DATA2_BIT)) a |= 0x04;
  if (!(DATA1_PIN & DATA1_BIT)) a |= 0x02;
  if (!(DATA0_PIN & DATA0_BIT)) a |= 0x01;
  return a;
}








//***********************************************
//*
//*  Check to see if data is available on the bus
//*   Revieves: VOID
//*     Returns: uint_8 zero if none ready, 1 if the bus has data
//*
//***********************************************

uint8_t GPIBDataReady(void)
{
    //Returns TRUE if it is asserted(LOW)
    //Returns FALSE if it is not asserted (HIGH)
    
    if (DAV_PIN & DAV_BIT) return 0;
    else return 1;
}








//***********************************************
//*
//*  Set the data pins on the GPIB bus
//*
//***********************************************
void  _WriteByte(uint8_t data)
{
// The DDR is already output when talk is initialized.  So simply drive the pins high or low



//WRITE DATA
//Call SerialSendStr("send >")
//Call SerialSendByte(data)
//Call SerialSendStr("<")
//Call SerialSendbyte(10)



  //set the pins individually bit by bit
 // check bits in data byte.  If a bit is set, then drive the data pin low otherwise, drive it high

  //DATA7_DDR |= DATA7_BIT;
  if (data & 0x80) DATA7_PORT &= ~DATA7_BIT;  //clear the pin if the bit is set
  else DATA7_PORT |= DATA7_BIT;                 //set the pin if the bit is clear

  //DATA6_DDR |= DATA6_BIT;
  if (data & 0x40) DATA6_PORT &= ~DATA6_BIT;  //clear the pin if the bit is set
  else DATA6_PORT |= DATA6_BIT;                 //set the pin if the bit is clear

  //DATA5_DDR |= DATA5_BIT;
  if (data & 0x20) DATA5_PORT &= ~DATA5_BIT;  //clear the pin if the bit is set
  else DATA5_PORT |= DATA5_BIT;                 //set the pin if the bit is clear

  //DATA4_DDR |= DATA4_BIT;
  if (data & 0x10) DATA4_PORT &= ~DATA4_BIT;  //clear the pin if the bit is set
  else DATA4_PORT |= DATA4_BIT;                 //set the pin if the bit is clear

  //DATA3_DDR |= DATA3_BIT;
  if (data & 0x08) DATA3_PORT &= ~DATA3_BIT;  //clear the pin if the bit is set
  else DATA3_PORT |= DATA3_BIT;                 //set the pin if the bit is clear

  //DATA2_DDR |= DATA2_BIT;
  if (data & 0x04) DATA2_PORT &= ~DATA2_BIT;  //clear the pin if the bit is set
  else DATA2_PORT |= DATA2_BIT;                 //set the pin if the bit is clear

  //DATA1_DDR |= DATA1_BIT;
  if (data & 0x02) DATA1_PORT &= ~DATA1_BIT;  //clear the pin if the bit is set
  else DATA1_PORT |= DATA1_BIT;                 //set the pin if the bit is clear

  //DATA0_DDR |= DATA0_BIT;
  if (data & 0x01) DATA0_PORT &= ~DATA0_BIT;  //clear the pin if the bit is set
  else DATA0_PORT |= DATA0_BIT;                 //set the pin if the bit is clear
}




//******************************************************************************
//******************************************************************************
//*   THIS IS THE LOW-LEVEL FUNCTION TO COMMUNICATE OVER THE GPIB BUS
//*   IT WILL RETURN AN ERROR IF THRE IS A TIMEOUT.
//*   TIMEOUT IS PERFORMED BY THE WAITFOR() FUNCTION WHICH RETURNS TRUE
//*   IF THERE WAS A TIMEOUT ERROR
//******************************************************************************
//******************************************************************************

uint8_t GPIBWriteByte(uint8_t datum, bool useEOI)
{
  uint8_t retval =0;
//place a byte onto the GPIB Bus with proper handshaking
//returns FALSE if no error, TRUE if there is an error

//debug.print "writebyte: >"+chr(datum)+"< = "+ Cstr(datum)
//debug.print "writebyte >"+chr(datum)+"< = "+ Cstr(datum)
//Serial.print (F(" Write Byte "));

//Call SerialSendStr("NDAC? ")
  retval = WaitForNDAC(_ASSERTED, 10000);
  if (retval)
  {
//Serial.print(F(" TIMEOUT NDAC ASSERT "));
        goto quit; //break;
  }
  
//Call SerialSendStr("NDAC OK, ")
  _WriteByte(datum);

//Call SerialSendStr("NRFD? ")
  retval = WaitForNRFD(_RELEASED, 10000);
  if (retval) 
  {
//Serial.print(F(" TIMEOUT NRFD RELEASE "));
        goto quit; //break;
  }
  
//Call SerialSendStr("NRFD OK, Set DAV, ")
  if(useEOI)
  {
    //Call SerialSendStr("Set EOI, ")
    GPIBAssert(&EOI_PORT, &EOI_DDR, EOI_BIT);
  }
  GPIBAssert(&DAV_PORT, &DAV_DDR, DAV_BIT);
  
//Call SerialSendStr("NDAC? ")
  retval = WaitForNDAC(_RELEASED, 10000);
  if (retval)  
  {
//Serial.print(F(" TIMEOUT NDAC RELEASE "));
        goto quit; //break;
  }

  quit:
  
//Call SerialSendStr("NDAC OK, BYTE DONE" +Chr(10)+chr(13))
  GPIBRelease(&DAV_PORT, &DAV_DDR, DAV_BIT);
  if(useEOI) GPIBRelease(&EOI_PORT, &EOI_DDR, EOI_BIT);  //maybe simple ALWAYS release EOI?

  return retval;
}











void GPIBSendState(void){
  
  uint8_t x = 0;
  uint8_t cnt = 0 ;
  
  Serial.println();
  Serial.print(F("DAV: "));
  if (DAV_PIN & DAV_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(DAV_DDR & DAV_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("NRFD: "));
  if (NRFD_PIN & NRFD_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(NRFD_DDR & NRFD_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("NDAC: "));
  if (NDAC_PIN & NDAC_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(NDAC_DDR & NDAC_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("EOI: "));
  if (EOI_PIN & EOI_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(EOI_DDR & EOI_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("DATA0: "));
  if (DATA0_PIN & DATA0_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(DATA0_DDR & DATA0_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("DATA1: "));
  if (DATA1_PIN & DATA1_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(DATA1_DDR & DATA1_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("DATA2: "));
  if (DATA2_PIN & DATA2_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(DATA2_DDR & DATA2_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("DATA3: "));
  if (DATA3_PIN & DATA3_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(DATA3_DDR & DATA3_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("DATA4: "));
  if (DATA4_PIN & DATA4_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(DATA4_DDR & DATA4_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("DATA5: "));
  if (DATA5_PIN & DATA5_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(DATA5_DDR & DATA5_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("DATA6: "));
  if (DATA6_PIN & DATA6_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(DATA6_DDR & DATA6_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
  Serial.print(F("DATA7: "));
  if (DATA7_PIN & DATA7_BIT) Serial.print(F("RELEASED "));
  else Serial.print(F("ASSERTED, "));
  if(DATA7_DDR & DATA7_BIT) Serial.println(F("OUTPUT"));
  else Serial.println(F("INPUT"));
  
}



















/*







'***********************************************
'***********************************************
Sub ReadGPIB()
' proceeds through a receive cycle as a state machine
' TODO: Add loop counter for timeout purposes

  Dim last as boolean = false 'true = no error
  

  Select Case receiveState
  Case 0
#ifdef DEBUG
Call SerialSendStr("RCV ")
#endif


    Call Clear(DAV) ' listener does not control DAV
    Call Assert(NDAC) ' data not accepted
    Call Release(NRFD)  ' ready for data
    if  CheckState(DAV)  then receiveState = 2' next state if DAV is already asserted
    receiveState = 1
    
  Case 1
    if  CheckState(DAV)  then receiveState = 2 'next state when DAV is asserted
    
    
  Case 2
    receivingData = true
    Call Assert(NRFD)   'indicate Not Ready For new Data
    if (CheckState(EOI)) then last = true 'check if last byte of transmission
    SerialSendByte(ReadByte())  'read the data and send the data
    Call Release(NDAC)
    receiveState = 3
    
  Case 3
    if (Not CheckState(DAV)) then receiveState = 4  'next state when DAV is released
    
  Case 4
    Call Assert(NDAC)   
    Call Release(NRFD)  'leaves line idle if this is the last byte
    if (last) then 
    SerialSendByte(_CR)
    SerialSendByte(_LF)
    Call Release(NDAC)  'leaves line idle
    End IF
    ReceiveState = 1
    receivingData = false
  
  End Select
    
    
end Sub






'***********************************************
'***********************************************
'Function WaitFor(byval line as Byte, byval state as Byte) as Byte
' Dim TimeOut as Long = 0
' Waitfor = 0 'ZERO indicates sucess
' PinRead() returns zero for low, and NON-ZERO for high...  Not simply a 1
' While ((IIF(PinRead(line)<>0,1,0) <> state) AND TimeOut < ctlTMOLoops)
'   TimeOut = TimeOut + 1
' Wend
' 
' if TimeOut = ctlTMOLoops then Waitfor = 1 'ONE indicates error
'End Function


















'*****************************************************
'*****************************************************
function CheckState(byval line as Byte) as Boolean

'  WARNING
' USING THIS FUNCTION ASSUMES THE PIN BEING CHECKED IS SET TO INPUT!!
' IF IT WASN'T "INPUT", GetPin() WILL MAKE IT SO!!!!

'checks to see if a line is asserted 
'Returns TRUE if it is asserted(LOW)
'Returns FALSE if it is not asserted (HIGH)
  If(GetPin(line) = 1) then 
  CheckState = false
  else 
  CheckState = true
  End If
  
End Function








'***********************************************
'***********************************************
'Function CheckDAV() as Boolean
''returns TRUE if GPIB bus is trying to send data
' CheckDAV = CheckState(DAV)
'End Function













sub AssertATN()
  Call Assert(ATN)
end sub

sub ReleaseATN()
  Call Release(ATN)
end sub




#ifdef DEBUG
#if DEBUG >=3
'**************************************
'
'
'*************************************
sub GPIBSendState()
dim x as byte = 0
dim cnt as byte =0 

'for x=1 to 10
'if IIF(PinRead(DAV)<>0,1,0)=1 then cnt = cnt+1 'DAV = 14
'next x
'call SerialSendStr(Chr(10)+Chr(13) + "DAV:")
'if (cnt = 0) then call SerialSendStr("ASSERTED ")
'if (cnt = 10) then  call SerialSendStr("RELEASED ")
'if ((cnt > 0) and (cnt < 10)) then call SerialSendStr("CHANGING ")
'cnt = 0

for x=1 to 10
if PinRead(NDAC)<>0 then cnt = cnt+1 
next x
call SerialSendStr("  NDAC:")
if (cnt = 0) then call SerialSendStr("ASSERTED ")
if (cnt = 10) then  call SerialSendStr("RELEASED ")
if ((cnt > 0) and (cnt < 10)) then call SerialSendStr("CHANGING ")
cnt = 0

for x=1 to 10
if PinRead(NRFD)<>0 then cnt = cnt+1 
next x
call SerialSendStr("  NRFD:")
if (cnt = 0) then call SerialSendStr("ASSERTED ")
if (cnt = 10) then  call SerialSendStr("RELEASED ")
if ((cnt > 0) and (cnt < 10)) then call SerialSendStr("CHANGING ")
cnt = 0

end sub
#endif
#endif










sub SetADDR(ByVal dat as byte)
  ctlAddr = dat
End Sub

function GetADDR() as byte
  GetADDR = ctlAddr
End Function


sub SetAUTO(ByVal dat as boolean)
  ctlAuto = dat
End Sub

function GetAUTO() as boolean
  GetAUTO = ctlAuto
End Function


sub SetEOI(ByVal dat as boolean)
  ctlEOI = dat
End Sub

function GetEOI() as boolean
  GetEOI = ctlEOI
End Function

sub SetTimeOut(ByVal dat as integer)
  ctlTimeOut = dat
End Sub

function GetTimeOut() as integer
  GetTimeOut = ctlTimeOut
End Function





'************************************************
'*
'*  CONFIGURATION
'*
'***********************************************

sub SaveCfg()
  'here the configuration will be saved to EEPROM
end sub

sub GetCfg()
  'here the configuration will be retrieved from EEPROM
  'and saved in the configuration variables
end sub

'************************************************
'*
'*  REN: remote enable
'*
'***********************************************

sub SetREN(ByRef arg as string)
  'valid args are:
  ' null (report back the setting)
  ' 0:  turn it off
  ' 1: turn it on
  
'Call SerialSendStr("REN arg: >")
'Call SerialSendStr(arg)
'Call SerialSendStr("<   ")

  if arg = "" then 
    SerialSendStr("REN reporting is not completed")
  elseif arg = "1" then 
   Call Assert(REN)
'  SerialSendStr("REN asserted")
   
  elseif arg = "2" then
    Call Release(REN)
'   SerialSendStr("REN released")
  end if
  
  'all other arg values are invalid and ignored
    
end sub



*/
