#include "Arduino.h"
#include "GPIB.h"
#include "tek_Interface.h"
#include "RTC.h"
#include "EEP.h"
#include <string.h>






#define IN_BUFFER 1050
//#define STR_BUFFER 100

extern RTC_PCF8523 rtc;


// try to use this data buffer for ALL scope I/O!!
char data_buffer[IN_BUFFER+1];  //make it just a little bigger than required for some protection
uint16_t ibcnt = 0;  //use a global variable that is consistent with NI488.1 as the number of bytes received


//define some frequently used commands:  ASSUMES SCOPE IS AT ADDRESS 1!
char DCL_Clear[] = {20,0};
char scopeL[] = {LADDR+ClientADDR, 0};  //IFC, untalk, unlisten, scope listens(0x20+address), null terminated
char scopeT[] = {TADDR+ClientADDR, 10, 0}; //unlisten, untalk, scope talks(0x40+address),   null terminated
char unt_unl[] = {UNT, UNL, 0};


//Setup some menus in flash
  const char menu0[] PROGMEM = "MESSAGE 1:\" SAVE    HIDE   UNUSED    SET   REBOOT\"";
  const char menu1[] PROGMEM = "MESSAGE 1:\" DATE    TIME    FILE            BACK\"";

//Create an array in flash of pointers to the strings in flash
  const char* const  menus[] PROGMEM = {menu0, menu1};

//access this with: 
//strcpy_P(data_buffer, (char *)pgm_read_word(&(menus[0]))); 




//*************************************************************************************************
//*
//*   Setup Tektronix interface and the GPIB interface it requires
//*
//*
//*************************************************************************************************
void tek_Setup(void) {
//Serial.println(F("starting Tek Interface"));
  GPIBInit();
  GPIBInterfaceClear();
//  GPIBInitTalk(); 
//Serial.println(F("Start Scope Init")); 
  tek_InitScope();
}







//*************************************************************************************************
//*
//*   Setup scope parameters to known states
//*
//*
//*************************************************************************************************
void tek_InitScope(void)
{
   //Send DCL to all devices (just one in this case)
 //  data_buffer[0] = 0x14;  //DCL
 //  GPIBWriteCmd(data_buffer, 1, true);

  //set scope to listen
  GPIBInitTalk();
  GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
  

//Serial.println(F("InitScope:sent command to listen scope"));



   
  //send the setup commands

#ifdef USE_SRQ
// SETUP FOR USING SRQ SIGNAL
   strcpy_P(data_buffer,PSTR("PATH OFF;LONG OFF;MENUOFF;USER ON;DAT ENC:RPB;WFM BN.F:RP;WFM ENC:BIN;RQS ON;INIT SRQ"));
#else
// SETUP FOR USING EVENT POLLING
   strcpy_P(data_buffer,PSTR("PATH OFF;LONG OFF;MENUOFF;USER ON;DAT ENC:RPB;WFM BN.F:RP;WFM ENC:BIN;RQS OFF;INIT SRQ"));
#endif

  GPIBWriteData (data_buffer, strlen(data_buffer), true);

  //set scope untalk, unlisten
  //GPIBWriteCmd(unt_unl, strlen(unt_unl), true);  //'true sets EOI at end
//Serial.println(F("Finished InitScope"));
  
}






//*************************************************************************************
//*
//*     displayedChannels  checks to determine which channels are displayed on the scope
//*
//*         Receives a pointer to an array large enough to hold NUMCHANNELS bytes
//          Returns: VOID, but fills the array with a 1 if the channel is displayed
//*                   or a zero if not displayed
//*
//*
//*************************************************************************************
void displayedChannels(uint8_t *list)
{
  //Check main waveforms CH1, CH2, ADD, MULT
  //Check saved waveforms REF1, REF2, REF3, REF4

  for (uint8_t channel=0; channel <NUMCHANNELS; channel++)
  {
      if (CheckChannel(channel)) list[channel]=1;
      else list[channel]=0;
  }
}




//********************************************************************************
//*
//*  Check Generic Channel if displayed
//*   Receives: channel name
//*   Returns: 1 if the channel is displayed, zero if not displayed
//*
//********************************************************************************
uint8_t CheckChannel(uint8_t channel) {
  uint8_t len;
  uint16_t recv;

  GPIBInitTalk();
// Tell the client to LISTEN for incoming data
  data_buffer[0]= 20; //DCL;
  data_buffer[1]= UNT;    //everybody un-talk
  data_buffer[2]= UNL;    //everybody un-listen
  data_buffer[3]= (ClientADDR + LADDR);
  GPIBWriteCmd(data_buffer, 4, false);  //'true sets EOI at end

// Send the client the commands
  data_buffer[0] = 'P';
  data_buffer[1] = 'A';
  data_buffer[2] = 'T';
  data_buffer[3] = 'H';
  data_buffer[4] = ' ';
  data_buffer[5] = 'O';
  data_buffer[6] = 'F';
  data_buffer[7] = 'F';
  GPIBWriteData (data_buffer, 8, true);

// Assemble the request stub
  switch (channel) {
    case CH1:
    case CH2:
    case ADD:
    case MULT:
      data_buffer[0] = 'V';
      data_buffer[1] = 'M';
      data_buffer[2] = 'O';
      data_buffer[3] = '?';
      data_buffer[4] = ' ';
      break;

    case REF1:
    case REF2:
    case REF3:
    case REF4:
      data_buffer[0] = 'R';
      data_buffer[1] = 'E';
      data_buffer[2] = 'F';
      data_buffer[3] = 'D';
      data_buffer[4] = 'I';
      data_buffer[5] = 'S';
      data_buffer[6] = 'P';
      data_buffer[7] = '?';
      data_buffer[8] = ' ';
      break;

   //default:
    //errorHandler(); 
  }

// Append the request ending
  switch (channel) {
    case CH1:
      data_buffer[5] = 'C';
      data_buffer[6] = 'H';
      data_buffer[7] = '1';
      len = 8;
      break;
      
    case CH2:
      data_buffer[5] = 'C';
      data_buffer[6] = 'H';
      data_buffer[7] = '2';
      len = 8;
      break;

    case ADD:
      data_buffer[5] = 'A';
      data_buffer[6] = 'D';
      data_buffer[7] = 'D';
      len = 8;
      break;

    case MULT:
      data_buffer[5] = 'M';
      data_buffer[6] = 'U';
      data_buffer[7] = 'L';
      len = 8;
      break;

    case REF1:
      data_buffer[9] = 'R';
      data_buffer[10] = 'E';
      data_buffer[11] = 'F';
      data_buffer[12] = '1';
      len = 13;
      break;

    case REF2:
      data_buffer[9] = 'R';
      data_buffer[10] = 'E';
      data_buffer[11] = 'F';
      data_buffer[12] = '2';
      len = 13;
      break;

    case REF3:
      data_buffer[9] = 'R';
      data_buffer[10] = 'E';
      data_buffer[11] = 'F';
      data_buffer[12] = '3';
      len = 13;
      break;

    case REF4:
      data_buffer[9] = 'R';
      data_buffer[10] = 'E';
      data_buffer[11] = 'F';
      data_buffer[12] = '4';
      len = 13;
      break;
  }

  GPIBWriteData (data_buffer,len, true);

//Tell the client to start talking
  data_buffer[0] = (ClientADDR + TADDR);
  data_buffer[1] = (10);
  GPIBWriteCmd(data_buffer, 2, true);

  GPIBInitListen();
  recv = Read_Data();  //read string from bus
  //Serial.print(F("The result length is "));
  //Serial.print (recv,DEC);
  //data_buffer[recv] = 0;  //make it null terminated
  //Serial.println((char*)data_buffer);

  //if (recv ==0) Serial.println (F("Errored out"));
  if ((data_buffer[0] == 'O') && (data_buffer[1] == 'N'))
  {
    //The channel is displayed.
    //Now get its data.
    //Serial.print(F("channel "));
    //Serial.print((uint8_t)channel,DEC);
   // Serial.println(F(" is displayed"));
    return 1;
  }
  else {
    //Serial.print(F("channel "));
    //Serial.print((uint8_t)channel,DEC);
    //Serial.println(F(" is NOT displayed"));
  }
  return 0;
}






//********************************************************************************
//*
//*     Read Waveform Preamble
//*         Sends commands to obtain the preamble, then parse out the extraneous characters
//*         Receives: channel name
//*         Returns: VOID, but the preamble text is in the data buffer
//*
//********************************************************************************
void ReadWfmPre(uint8_t channel)
{
  uint16_t recv;
  uint8_t len;

  GPIBInitTalk();
// Tell the client to LISTEN for incoming data
  data_buffer[0]= 20; //DCL;
  data_buffer[1]= UNT;    //everybody un-talk
  data_buffer[2]= UNL;    //everybody un-listen
  data_buffer[3]= (ClientADDR + LADDR);
  GPIBWriteCmd(data_buffer, 4, false);  //'true sets EOI at end

// Send the client the basic setup commands to be sure we know how it will respond
  data_buffer[0] = 'P';
  data_buffer[1] = 'A';
  data_buffer[2] = 'T';
  data_buffer[3] = 'H';
  data_buffer[4] = ' ';
  data_buffer[5] = 'O';
  data_buffer[6] = 'F';
  data_buffer[7] = 'F';
  GPIBWriteData (data_buffer, 8, true);

  //set the data encoding mode
  data_buffer[0] = 'D';
  data_buffer[1] = 'A';
  data_buffer[2] = 'T';
  data_buffer[3] = ' ';
  data_buffer[4] = 'E';
  data_buffer[5] = 'N';
  data_buffer[6] = 'C';
  data_buffer[7] = ':';
  data_buffer[8] = 'R';
  data_buffer[9] = 'P';
  data_buffer[10] = 'B';
  GPIBWriteData (data_buffer, 11, true);

  data_buffer[0] = 'D';
  data_buffer[1] = 'A';
  data_buffer[2] = 'T';
  data_buffer[3] = ' ';
  data_buffer[4] = 'S';
  data_buffer[5] = 'O';
  data_buffer[6] = 'U';
  data_buffer[7] = ':';

  switch (channel) {
    case CH1:
      data_buffer[8] = 'C';
      data_buffer[9] = 'H';
      data_buffer[10] = '1';
      len =11;
      break;
      
    case CH2:
      data_buffer[8] = 'C';
      data_buffer[9] = 'H';
      data_buffer[10] = '2';
      len =11;
      break;
      
    case ADD:
      data_buffer[8] = 'A';
      data_buffer[9] = 'D';
      data_buffer[10] = 'D';
      len =11;
      break;

    case MULT:
      data_buffer[8] = 'M';
      data_buffer[9] = 'U';
      data_buffer[10] = 'L';
      len =11;
      break;

    case REF1:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '1';
      len =12;
      break;

    case REF2:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '2';
      len =12;
      break;

    case REF3:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '3';
      len =12;
      break;

    case REF4:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '4';
      len =12;
      break;

    //default:
      //errorFunction();
  }

  GPIBWriteData (data_buffer, len, true);

  //Now request the waveform preamble
  data_buffer[0] = 'W';
  data_buffer[1] = 'F';
  data_buffer[2] = 'M';
  data_buffer[3] = '?';
  GPIBWriteData (data_buffer, 4, true);

//Tell the client to start talking
  data_buffer[0] = (ClientADDR + TADDR);
  data_buffer[1] = (10);
  GPIBWriteCmd(data_buffer, 2, true);
  
 GPIBInitListen();
 recv = Read_Data();  //read string from bus
  //if (recv ==0) Serial.println (F("Errored out"));
  //else
  if (recv != 0)
  {
    ParseWFM();
    //Serial.write(data_buffer);
    //Serial.println();
  }
}








//********************************************************************************
//*
//*     Read Curve
//*         Sends commands to obtain the curve data, then parse out the extraneous characters
//*         Receives: channel name
//*         Returns: VOID, but the data is in the data buffer
//*
//********************************************************************************
void ReadCurve(uint8_t channel)
{
  uint16_t recv;
  uint8_t len;

  GPIBInitTalk();
// Tell the client to LISTEN for incoming data
  data_buffer[0]= 20; //DCL;
  data_buffer[1]= UNT;    //everybody un-talk
  data_buffer[2]= UNL;    //everybody un-listen
  data_buffer[3]= (ClientADDR + LADDR);
  GPIBWriteCmd(data_buffer, 4, false);  //'true sets EOI at end

// Send the client the basic setup commands to be sure we know how it will respond
  data_buffer[0] = 'P';
  data_buffer[1] = 'A';
  data_buffer[2] = 'T';
  data_buffer[3] = 'H';
  data_buffer[4] = ' ';
  data_buffer[5] = 'O';
  data_buffer[6] = 'F';
  data_buffer[7] = 'F';
  GPIBWriteData (data_buffer, 8, true);

  //set the data encoding mode
  data_buffer[0] = 'D';
  data_buffer[1] = 'A';
  data_buffer[2] = 'T';
  data_buffer[3] = ' ';
  data_buffer[4] = 'E';
  data_buffer[5] = 'N';
  data_buffer[6] = 'C';
  data_buffer[7] = ':';
  data_buffer[8] = 'R';
  data_buffer[9] = 'P';
  data_buffer[10] = 'B';
  GPIBWriteData (data_buffer, 11, true);

  data_buffer[0] = 'D';
  data_buffer[1] = 'A';
  data_buffer[2] = 'T';
  data_buffer[3] = ' ';
  data_buffer[4] = 'S';
  data_buffer[5] = 'O';
  data_buffer[6] = 'U';
  data_buffer[7] = ':';

  switch (channel) {
    case CH1:
      data_buffer[8] = 'C';
      data_buffer[9] = 'H';
      data_buffer[10] = '1';
      len =11;
      break;
      
    case CH2:
      data_buffer[8] = 'C';
      data_buffer[9] = 'H';
      data_buffer[10] = '2';
      len =11;
      break;
      
    case ADD:
      data_buffer[8] = 'A';
      data_buffer[9] = 'D';
      data_buffer[10] = 'D';
      len =11;
      break;

    case MULT:
      data_buffer[8] = 'M';
      data_buffer[9] = 'U';
      data_buffer[10] = 'L';
      len =11;
      break;

    case REF1:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '1';
      len =12;
      break;

    case REF2:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '2';
      len =12;
      break;

    case REF3:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '3';
      len =12;
      break;

    case REF4:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '4';
      len =12;
      break;

    //default:
      //errorFunction();
  }

  GPIBWriteData (data_buffer, len, true);

  
  //Now get the curve
  data_buffer[0] = 'C';
  data_buffer[1] = 'U';
  data_buffer[2] = 'R';
  data_buffer[3] = 'V';
  data_buffer[4] = '?';
  GPIBWriteData (data_buffer, 5, true);

  //Tell the client to start talking
  data_buffer[0] = (ClientADDR + TADDR);
  data_buffer[1] = (10);
  GPIBWriteCmd(data_buffer, 2, true);

  GPIBInitListen();
  recv = Read_Data();  //read string from bus
  //if (recv ==0) Serial.println (F("Errored out"));
  //else
  if (recv != 0)
  {
    //Serial.print(F("Data received: "));
    //Serial.print(recv);
    //Serial.println(F("bytes"));


    //now do something with the data
    ParseData();
    //now send the data the the flash drive
  }
}






/*
//********************************************************************************
//*
//*  Read Generic Channel
//*   Receives: channel name
//*   Returns: void
//*
//********************************************************************************
void ReadChannel(uint8_t channel)
{
  uint16_t recv;
  uint8_t len;

  GPIBInitTalk();
// Tell the client to LISTEN for incoming data
  data_buffer[0]= 20; //DCL;
  data_buffer[1]= UNT;    //everybody un-talk
  data_buffer[2]= UNL;    //everybody un-listen
  data_buffer[3]= (ClientADDR + LADDR);
  GPIBWriteCmd(data_buffer, 4, false);  //'true sets EOI at end

// Send the client the basic setup commands to be sure we know how it will respond
  data_buffer[0] = 'P';
  data_buffer[1] = 'A';
  data_buffer[2] = 'T';
  data_buffer[3] = 'H';
  data_buffer[4] = ' ';
  data_buffer[5] = 'O';
  data_buffer[6] = 'F';
  data_buffer[7] = 'F';
  GPIBWriteData (data_buffer, 8, true);

  //set the data encoding mode
  data_buffer[0] = 'D';
  data_buffer[1] = 'A';
  data_buffer[2] = 'T';
  data_buffer[3] = ' ';
  data_buffer[4] = 'E';
  data_buffer[5] = 'N';
  data_buffer[6] = 'C';
  data_buffer[7] = ':';
  data_buffer[8] = 'R';
  data_buffer[9] = 'P';
  data_buffer[10] = 'B';
  GPIBWriteData (data_buffer, 11, true);

  data_buffer[0] = 'D';
  data_buffer[1] = 'A';
  data_buffer[2] = 'T';
  data_buffer[3] = ' ';
  data_buffer[4] = 'S';
  data_buffer[5] = 'O';
  data_buffer[6] = 'U';
  data_buffer[7] = ':';

  switch (channel) {
    case CH1:
      data_buffer[8] = 'C';
      data_buffer[9] = 'H';
      data_buffer[10] = '1';
      len =11;
      break;
      
    case CH2:
      data_buffer[8] = 'C';
      data_buffer[9] = 'H';
      data_buffer[10] = '2';
      len =11;
      break;
      
    case ADD:
      data_buffer[8] = 'A';
      data_buffer[9] = 'D';
      data_buffer[10] = 'D';
      len =11;
      break;

    case MULT:
      data_buffer[8] = 'M';
      data_buffer[9] = 'U';
      data_buffer[10] = 'L';
      len =11;
      break;

    case REF1:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '1';
      len =12;
      break;

    case REF2:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '2';
      len =12;
      break;

    case REF3:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '3';
      len =12;
      break;

    case REF4:
      data_buffer[8] = 'R';
      data_buffer[9] = 'E';
      data_buffer[10] = 'F';
      data_buffer[11] = '4';
      len =12;
      break;

    //default:
      //errorFunction();
  }

  GPIBWriteData (data_buffer, len, true);

  //Now request the waveform preamble
  data_buffer[0] = 'W';
  data_buffer[1] = 'F';
  data_buffer[2] = 'M';
  data_buffer[3] = '?';
  GPIBWriteData (data_buffer, 4, true);

//Tell the client to start talking
  data_buffer[0] = (ClientADDR + TADDR);
  data_buffer[1] = (10);
  GPIBWriteCmd(data_buffer, 2, true);

 recv = Read_Data();  //read string from bus
  if (recv ==0) Serial.println (F("Errored out"));
  else
  {
    ParseWFM();
    Serial.write(data_buffer);
    Serial.println();
  }
  data_buffer[recv] = 0;  //place a null at the end of the data

  //now do the curve data
  // Tell the client to LISTEN for incoming data
  GPIBInitTalk();
  data_buffer[0]= 20; //DCL;
  data_buffer[1]= UNT;    //everybody un-talk
  data_buffer[2]= UNL;    //everybody un-listen
  data_buffer[3]= (ClientADDR + LADDR);
  GPIBWriteCmd(data_buffer, 4, false);  //'true sets EOI at end

  //get the curve
  data_buffer[0] = 'C';
  data_buffer[1] = 'U';
  data_buffer[2] = 'R';
  data_buffer[3] = 'V';
  data_buffer[4] = '?';
  GPIBWriteData (data_buffer, 5, true);

  //Tell the client to start talking
  data_buffer[0] = (ClientADDR + TADDR);
  data_buffer[1] = (10);
  GPIBWriteCmd(data_buffer, 2, true);

  recv = Read_Data();  //read string from bus
  if (recv ==0) Serial.println (F("Errored out"));
  else
  {
    Serial.print(F("Data received: "));
    Serial.print(recv);
    Serial.println(F("bytes"));


    //now do something with the data
    ParseData();
    //now send the data the the flash drive
  }
}
*/



//******************************************************************************
//*
//*   Parse WFMPre
//*
//******************************************************************************

uint8_t ParseWFM(void)
{
    //find the text within quotes in the WFM string
    //and move it to the beginning and null terminate it
    
    uint16_t pos1, pos2;
    uint16_t  x, index;
    char* ptr;
    
    
    ptr = strchr(data_buffer,'"');
    pos1 = ptr-data_buffer;
    //Serial.println(pos1);
    ptr = strchr(&data_buffer[pos1+1],'"');
    pos2 = ptr-data_buffer;
    for (x = pos1+1, index = 0; x < pos2; x++, index++) data_buffer[index] = data_buffer[x];
    data_buffer[index] = 0; //null terminate the new string
    return (uint8_t) index+1;
}






//******************************************************************************
//*
//*   Parse Data
//*
//******************************************************************************

void ParseData(void)
{
    //The data has 3 bytes of a header in the beginning
    //And one byte of a checksum at the end
    //So, move the data forward by 3 bytes to eliminate the header
    //The header is one byte of a special character, then 2 bytes of the data size in case it matters

    //The starting point is byte 3 and the number of bytes to move is 1024


//TODO: Use memmove()?  movemem()?    
    uint16_t  x, index;
    for (x = 0, index = 3; x < 1024; x++, index++) data_buffer[x] = data_buffer[index];
    //data_buffer[1024] = 0; //put a null at the end just to show where the end is.
}





//******************************************************************************
//*
//*   Get Data from bus
//*       returns the number of bytes received
//*       and saves the data in a manner compatible with NI-488.1 standard: ibcnt
//*
//******************************************************************************

uint16_t Read_Data(void)
{
  //GPIBInitListen();
  while(!GPIBDataReady()); //wait for something to happen on the bus
  ibcnt = GPIBGetData(data_buffer, IN_BUFFER, 10000);
  return ibcnt;
}



//******************************************************************************
//*
//*  Get Event from scope after SRQ is asserted
//*
//******************************************************************************

uint16_t GetEvent_SRQ(void)
{
  uint16_t datalen;
  uint16_t recv;
  
//TODO:  Consider saving the status byte obtained from polling into a global variable

  /*First unaddress bus devices and and send
Serial Poll Enable (SPE) command, followed by scope's talk address, and GPIB
board's listen address. */
/*ibcmd (bd,"?_\x18G ",5);*/      /*UNL UNT SPE TAD MLA*/

//perform serial poll action to clear the pending SRQ status bit
  data_buffer[0] = UNL;
  data_buffer[1] = UNT;
  data_buffer[3] = SPE_EN;               //enable serial poll mode
  data_buffer[2] = TADDR+ClientADDR;  //tell the scope to talk, it will respond with the status byte
  GPIBInitTalk();
//Serial.println(F("about to write SPOLL command"));
  GPIBWriteCmd(data_buffer, 4, false);  //'true sets EOI at end
//  data_buffer[0] = 0; //clear out some bytes as a test
//  data_buffer[1] = 0;
  
//Serial.println(F("command sent, reading result"));  
  GPIBInitListen();
  //Read_Data();
  ibcnt = GPIBGetData(data_buffer, 1, 10000); //read precisely ONE byte using handshaking

//  Serial.print(F("spoll result length:"));
//  Serial.println(ibcnt);
  data_buffer[ibcnt] = 0;
//  Serial.print(F("spoll result string:"));
//  Serial.println(data_buffer);
//  Serial.print(F("spoll result value:"));
//  Serial.println(byte(data_buffer[0]));

  data_buffer[0] = SPD;               //disable serial poll mode
  data_buffer[1] = UNT;
  data_buffer[2] = UNL;
  GPIBInitTalk();
  GPIBWriteCmd(data_buffer, 3, true);  //'true sets EOI at end

  
// now read the pending event  
  strcpy_P(data_buffer,PSTR("EVENT?"));

  //set scope to listen
  GPIBInitTalk();
  GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
   
  //send the command
  GPIBWriteData (data_buffer, strlen(data_buffer), true);

  //set scope to talk
  GPIBWriteCmd(scopeT, strlen(scopeT), true);  //'true sets EOI at end

  //read the event code
  GPIBInitListen();
   Read_Data();  //Puts data into data_buffer array
   
//   if (ibcnt >=4)  //check for invalid data: event code should be a string no more than 3 characters
//   {
//     Serial.println(F("data overrun"));
     //return 0;
//   }
//   Serial.print(F("event length is:"));
//   Serial.println(ibcnt);
   data_buffer[ibcnt] = 0; //null terminate the end of the received characters to create a string 
   recv = atoi(data_buffer);
//   Serial.print(F("event string is:"));
//   Serial.println(data_buffer);
//   Serial.print(F("event code is:"));
//   Serial.println(recv);

   return recv;
   
}





//******************************************************************************
//*
//*  Get Event from scope by polling event
//*
//******************************************************************************

uint16_t GetEvent(void)
{
    uint16_t recv;


  
  //read the pending event  
  strcpy_P(data_buffer,PSTR("EVENT?"));

  //set scope to listen
  GPIBInitTalk();
  GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
   
  //send the command
  GPIBWriteData (data_buffer, strlen(data_buffer), true);

  //set scope to talk
  GPIBWriteCmd(scopeT, strlen(scopeT), true);  //'true sets EOI at end
    
   //read the event code
   GPIBInitListen();
   Read_Data();  //Puts data into data_buffer array
   
//   if (ibcnt >=4)  //check for invalid data: event code should be a string no more than 3 characters
//   {
//     Serial.println(F("data overrun"));
     //return 0;
//   }

//   Serial.print(F("event length is:"));
//   Serial.println(ibcnt);
   data_buffer[ibcnt] = 0; //null terminate the end of the received characters to create a string 
   recv = atoi(data_buffer);
//   Serial.print(F("event string is:"));
//   Serial.println(data_buffer);
//   Serial.print(F("event code is:"));
//   Serial.println(recv);

   return recv;
}





//******************************************************************************
//*
//*  Show intro screen on the sscope
//*       save some current settings
//*
//******************************************************************************
void introScreen(void)
{
   
    char dispInten[50]="";
    char trigMode[50] ="";
    char runMode[50]="";
    uint16_t datalen;
    DateTime now;
    char buf[10];

    
//GPIBInterfaceClear();
//Serial.print(F("intro screen:"));
    //*** read some basic settings
    //*** store them, and later restore them
    //*** This is to stop any acquisitions and blank the waveform display

    GPIBInitTalk();
    GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end

    strcpy_P(data_buffer,PSTR("PATH ON; INTENSITY? DISPLAY"));
    GPIBWriteData(data_buffer, strlen(data_buffer), true);
    
    //ibcmd(bd, scopt, strlen(scopt));
    GPIBWriteCmd(scopeT, strlen(scopeT), true);  //'true sets EOI at end
    //ibrd(bd,mess, 60);
    GPIBInitListen();
    Read_Data();  //Puts data into data_buffer array
    if (ibcnt >0)
    {
       data_buffer[ibcnt] = 0;
       strcpy(dispInten,data_buffer);
    }

    GPIBInitTalk();
    strcpy_P(data_buffer,PSTR("RUN?"));
    GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    GPIBWriteCmd(scopeT, strlen(scopeT), true);  //'true sets EOI at end
    GPIBInitListen();
    Read_Data();  //Puts data into data_buffer array
    if (ibcnt> 0)
    {
      data_buffer[ibcnt]=0;
      strcpy(runMode,data_buffer);
    }

    GPIBInitTalk();
    strcpy_P(data_buffer,PSTR("ATR? MODE"));
    GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    GPIBWriteCmd(scopeT, strlen(scopeT), true);  //'true sets EOI at end
    GPIBInitListen();
    Read_Data();  //Puts data into data_buffer array
    if(ibcnt>0)
    {
      data_buffer[ibcnt]=0;
      strcpy(trigMode,data_buffer);
    }


 

    //*** now send the intro screen ***
    GPIBInitTalk();
    GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
    strcpy_P(data_buffer, PSTR("INTENSITY DISP: 0; ATR MOD: SGL; MEN; MESS CLR"));
    //printf("sending :\"%s\"\r\n",data_buffer);
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    strcpy_P(data_buffer, PSTR("READOUT ON; MESSAGE CLRSTATE; MENUOFF"));
    //printf("sending:\"%s\"\r\n",data_buffer);
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    strcpy_P(data_buffer, PSTR("MESSAGE 12:\"TEKTRONICS GRAPHICS SOFTWARE\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    strcpy_P(data_buffer, PSTR("MESSAGE 11:\"COPYRIGHT -C- 2019 ANTHONY RHODES\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    
//access RTC functions
    now = rtc.now();
    strcpy_P(data_buffer,PSTR("MESSAGE 10:\"  DATE: "));
    itoa(now.month(), buf, 10);
    strFix(buf);
    //sprintf_P(buf, PSTR("%02d"), now.month());
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR("/"));
    itoa(now.day(), buf, 10);
    strFix(buf);
    //sprintf_P(buf, PSTR("%02d"), now.day());
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR("/"));
    itoa(now.year(),buf , 10);
    //strFix(buf); should not need to fix the year, just use it as is
    //sprintf_P(buf, PSTR("%04d"), now.year());
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR("\""));  //add the non-displayed terminator
Serial.println(data_buffer);
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    
    strcpy_P(data_buffer,PSTR("MESSAGE 9:\"  TIME: "));
    itoa(now.hour(), buf, 10);
    strFix(buf);
    //sprintf_P(buf, PSTR("%02d"), now.hour());
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR(":"));
    itoa(now.minute(), buf, 10);
    strFix(buf);
    //sprintf_P(buf, PSTR("%02d"), now.minute());
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR(":"));
    itoa(now.second(), buf, 10);
    strFix(buf);
    //sprintf_P(buf, PSTR("%02d"), now.second());
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR("\""));  //add the non-displayed terminator
Serial.println(data_buffer);
    GPIBWriteData (data_buffer, strlen(data_buffer), true);

    //Get the file name stub from EEPROM
    EEP_ReadNameStub(buf);
    strcpy_P(data_buffer,PSTR("MESSAGE 8:\"  FILE NAME: >"));
    strcat(data_buffer, buf);
    strcat_P(data_buffer,PSTR("<\""));
Serial.println(data_buffer);
    GPIBWriteData (data_buffer, strlen(data_buffer), true);


    
    
    delay(10000);  //delay 8 seconds... LESS?

    //*** restore display everything to the way it was
    GPIBWriteData(dispInten,strlen(dispInten), true);
    GPIBWriteData(trigMode, strlen(trigMode), true);
    GPIBWriteData(runMode, strlen(runMode), true);

    //restore settings
    strcpy_P(data_buffer,PSTR("MESSAGE CLRSTATE; MENUOFF; READOUT ON;PATH OFF"));
    GPIBWriteData(data_buffer, strlen(data_buffer), true);



//    Serial.println(F("exiting introScreen"));
}





//*********************************************
//*
//*     strFix
//*
//*      lightweight function to re-format a numeric month/day/hour/minute/second
//*      for use with itoa() which will not pad leading zeros
//*      This will pad a single digit string with a leading zero if needed
//*      Saves over 1K flash compared to sprintf_P(), and must be faster too, but not checked
//*
//*********************************************
void strFix(char * buf) {
  //if the second character is null, then it needs to be padded with a leading zero
  if (buf[1] == 0) {
    buf[2] = 0;
    buf[1] = buf[0];
    buf[0]='0';
  }
}





//*****************************************************************
//*
//*     CheckID
//*         Queries the scope for it ID
//*         It then compares that to several model numbers
//*         returns TRUE for an occurrence of one of the model numbers, or
//*         FALSE for no occurence of the model numbers
//*
//*****************************************************************
uint8_t checkID(void)
{
    uint8_t result=0;
    //char IDstr[5];
    
    GPIBInitTalk();
    strcpy_P(data_buffer,PSTR("ID?"));
    GPIBWriteCmd(scopeL, strlen(scopeL), false);
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    GPIBWriteCmd(scopeT, strlen(scopeT), true);
    GPIBInitListen();
    Read_Data();
    if(ibcnt>0)
    {
      data_buffer[ibcnt]=0;
//      Serial.print(F("ID:"));  
//      Serial.print(data_buffer);
//      Serial.println("<");
      if (strstr_P(data_buffer, PSTR("2430"))) result = 1;
      else if (strstr_P(data_buffer, PSTR("2432"))) result = 1;
      else if (strstr_P(data_buffer, PSTR("2440"))) result = 1;
    }
//Serial.print(F("ID Result:"));
//Serial.println(result);
//Serial.print(F("Length:"));
//Serial.println(ibcnt);
    return result;
}






//***************************************************************
//*
//*   Send a message to the scope (non-progmem version)
//*      Places the message on line 3
//*      TODO: write a progmen version!
//*
//***************************************************************
void tek_Message(const char *buf)
{
     char msg[60];
    
     strcpy_P(msg, PSTR("MESSAGE 3:\"")); //11 characters in the non-displayed leader
     strncat (msg,buf,40);  //copy maximum of 40 characters to be displayed on the scope
     strcat_P(msg,PSTR("\""));  //add the non-displayed terminator

     GPIBInitTalk();
     GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
     GPIBWriteData (msg, strlen(msg), true);
}



//***************************************************************
//*
//*   Ring the bell on the scope
//*
//***************************************************************
void tek_RingBell(void)
{
     char msg[] = "BEL";

     GPIBInitTalk();
     GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
     GPIBWriteData (msg, strlen(msg), true);
}






//***************************************************************
//*
//*   Hide and Unhide the function menu
//*      receives an optional argument of "force" which will force display
//*      and set the state accordingly
//*      Must use menu strings setup in the beginning of this modula as globals
//*
//***************************************************************
void tek_Menu(uint8_t state, uint8_t force)
{
  static uint8_t hidden = false;

    if (force) hidden = true;  //force the state to show the menu line
    
    if (hidden == true) { // if currently hidden, toggle status and show the menu line
      hidden = false;
      GPIBInitTalk();
      strcpy_P(data_buffer, (char *)pgm_read_word(&(menus[state])));
      GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end 
      GPIBWriteData (data_buffer, strlen(data_buffer), true);
    }
    else {    // if currently showing, toggle status and clear the menu line
      hidden = true;
      strcpy_P(data_buffer, PSTR("MESSAGE 1:\"\"")); //clear the menu line
      GPIBInitTalk();
      GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
      GPIBWriteData (data_buffer, strlen(data_buffer), true);
    }  //end if
}




void    tek_SetFileName(void)
{
    //#define startPos 1
    #define NAMELEN 5   //set the length of the name stub:  5 characters with 3 more digit serialization.  Could be 6+2 instead
    #define NAMEPOS 30  //position of the first character of the name in the scope line
    #define EMPTY 37     //special scope character signifying an empty character cell (37 = %)
    #define ARROW 94    //special scope character showing an arrow
    
    char buf[8] ={0,0,0,0,0,0,0,0};
    int8_t pointer = 0;  //position of letter currently being changed
    uint8_t i;    //length of the file name stub
    uint16_t event;
    uint8_t done = false;
    char ch;

    // display characers of the name with special character for non-characters e.g."TEK--"
    // ascii "0" = 48d, "9" = 57d, "A" = 65d, "Z" = 90d, "_" = 95 "-" = 45d

    GPIBInitTalk();
    GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
    //read existing name
    EEP_ReadNameStub(buf);  //place the current name stub in the buffer
    
    //consider checking that buf is made of all valid characters for a file name....
    //can't use zero as a character, so convert zeros to "EMPTY".  UNDO this at the end
    for (i=0; i<NAMELEN; i++){
        if (buf[i] == 0) buf[i] = EMPTY;
    }
    buf[NAMELEN]=0; //null delimiter after the last possible character

    //set the menu line
    strcpy_P(data_buffer, PSTR("MESSAGE 1:\"                   UP    DOWN    SET\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    
    while (!done)
    {
        //show the filename and pointer
        strcpy_P(data_buffer,PSTR("Message 6:\"SET THE FILE NAME: "));
        strcat(data_buffer,buf);
        strcat_P(data_buffer,PSTR("nnn.svg\""));
        GPIBWriteData (data_buffer, strlen(data_buffer), true);
// Serial.println(data_buffer);
        strcpy_P(data_buffer,PSTR("Message 5:\"                           \""));
        data_buffer[NAMEPOS+pointer] = '\\';
        data_buffer[NAMEPOS+pointer+1] = ARROW;
// Serial.println(data_buffer);
        GPIBWriteData (data_buffer, strlen(data_buffer), true);


        //wait for SRQ
        while (WaitForSRQ(0, 0xFFFE));  //returns a 1 if timeout occurs.  Keep polling SRQ until it is asserted (LOW): returns before timeout with a value of zero.
        //read event  
        event = GetEvent_SRQ();
           
        //then need to restore the controller pins to talk status and scope to listen status!!!
        GPIBInitTalk();
        GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
           
        //if event is up then increment setting[i], bounds check, wrap
        if (event == 452) { //UP
//Serial.println(F("UP...")); //A-Z, 0-9, -, EMPTY in that order (NO underscore allowed by scope)
            //roll the buffer character upward
//Serial.println((uint8_t) buf[pointer]);
            buf[pointer]++;
            if (buf[pointer] == 'Z'+1) {
                  buf[pointer] = '0'; 
//Serial.println((uint8_t) buf[pointer]);
            }
            else if (buf[pointer] == '9'+1) buf[pointer] = '-'; //9 = 57
            else if (buf[pointer] == '-'+1) 
            {
                if (pointer==0) buf[pointer] = 'A'; //the first character of the filename MUST be a valid character
                else buf[pointer] = EMPTY;
            }
            else if (buf[pointer] == EMPTY+1) {
//Serial.println((uint8_t) buf[pointer]);
                buf[pointer] = 'A';
//Serial.println(F("increment up from empty (43?)"));
//Serial.println((uint8_t) buf[pointer]);
            }
        }
        else if (event == 453) {//DOWN
            //roll the buffer character downward
            buf[pointer]--;
            if (buf[pointer] == '-'-1) buf[pointer] = '9';
            else if (buf[pointer] == '0'-1) buf[pointer] = 'Z';
            else if (buf[pointer] == 'A'-1) 
            {
                if (pointer==0) buf[pointer] = '-'; //the first character of the filename MUST be a valid character
                else buf[pointer] = EMPTY;
            }
            else if (buf[pointer] == EMPTY-1) buf[pointer] = '-';
             
        }

        else if (event == 454) {//set
            pointer++;
            if (pointer == NAMELEN) done = true;
        }
    } //end while !done

    //now the file name stub is 0 to 5 characters, with 5 to 0 EMPTY characters
    //scan through the name buffer and convert all EMPTY characters to a null
    for (i = 0; i<NAMELEN; i++)
    {
        if (buf[i] == EMPTY) buf[i] = 0;    
    }
    // check for invalid filename.  If so, don't save it!
    if (buf[0] == 0) 
    {
        buf[0] = 'A';  //prevent a null file name! for now until better error checking
        buf[1] = 0;    //the second character is undefined.  Define it as a null
    }
       


    strcpy_P(data_buffer,PSTR("Message 6:\"SAVING FILE NAME: "));
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR("nnn.svg\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
//Serial.println(data_buffer);
    EEP_WriteNameStub(buf);

    strcpy_P(data_buffer,PSTR("Message 5:\"\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    delay(4000);  //wait for 4 seconds
    //erase the remaining scope screen line
    strcpy_P(data_buffer,PSTR("Message 6:\"\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    
}





/*
//**************************************************************
//*  
//*  My own fast atoi() function
//*      assumes charaters are only numeric - NO CHECKING!
//*      Null terminated string
//*
//*************************************************************
uint16_t my_atoi(const char *str)
{
    uint16_t val = 0;
    while( *str ) {
        val = val*10 + (*str++ - '0');
    }
    return val;
}
*/
