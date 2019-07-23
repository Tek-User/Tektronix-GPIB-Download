#include <Wire.h>
#include "tek_Interface.h"
#include "GPIB.h"
#include "RTC.h"





RTC_PCF8523 rtc;
extern char data_buffer[];  //defined in Tek_Interface module
extern char scopeL[]; //defined in Tek_Interface module


/*
TO IMPLEMENT THE FILE DATE/TIME FUNCTION, PLACE THE FOLLOWING
IN SVG.CPP PLACE THE CALL BACK FUNCTION A THE TOP OF THE CODE 
AND ADD THE RTCLIB.H HEADER TOO!

extern RTC_PCF8523 rtc; //defined in RTC.cpp

void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}


THEN REFERENCE THE CALL BACK NEAR THE END JUST BEFORE THE LINE
"TEKfile = SD.open(filename, FILE_WRITE); "
ADD THE LINE 
" SdFile::dateTimeCallback(dateTime);"


HOWEVER, I THINK THAT THE SdFile::... LINE AN BE EXECUTED ONCE ND NEVER NEEDS IT AGAIN.  aND IT COULD BE IN A FUNCTION
LOCATED IN SVG.CPP CALLED SVG_SETUP(VOID) {
SdFile::dateTimeCallback(dateTime);
}

AND MAYBE THAT IS THE ONLY THING IT DOES  ANLL OTHER SETUP STUFF IS DONE SEPARATELY
AND DO NOT USE IT JUST BEFORE OPENING THE FILE.  IT SIMPLPE NEEDS TO BE DOONCE BEFORE THE FIRST TIME OPENNG THE SD FILE


*/

 







//***************************************************************
//*
//*     Initialize the I2C bus
//*
//***************************************************************
void RTC_setup(void) {
  
Serial.println(F("setting up RTC"));

    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
   }              // join i2c bus (address optional for master)


 if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");  //therefore the date and time are invalid
    strcpy_P(data_buffer,PSTR("RTC ERROR: USING DEFAULT TIME"));
    tek_RingBell();
    tek_Message(data_buffer); //send a message to the scope screen indicating a problem
    delay(4000);
    data_buffer[0];
    tek_Message(data_buffer);
    
    rtc.adjust(DateTime(2019, 1, 1, 8, 0, 0)); //DEFAULT DATE & TIME IS 8AM on 1.1.2019
  }

}






//***************************************************************
//*
//*    RTC_GetRTCDate(uint16_t &year, uint8_t &month, unit8_t &day)
//*        Returns the year, month, day
//*
//***************************************************************
void  RTC_GetRTCDate(uint16_t &year, uint8_t &month, uint8_t &day) {
    DateTime now = rtc.now();
    year = now.year();
    month = now.month();
    day = now.day();
}




//***************************************************************
//*
//*    RTC_SetRTCDate(uint16_t &year, uint8_t &month, unit8_t &day)
//*        sets the year, month, day
//*
//***************************************************************
void  RTC_SetRTCDate(uint16_t &year, uint8_t &month, uint8_t &day) {
    DateTime now = rtc.now();
    rtc.adjust(DateTime(year, month, day, now.hour(), now.minute(), now.second()));
    
}



//***************************************************************
//*
//*    RTC_SetDate(void)
//*        Interface with scope to manage date and time
//*        sets the RTC year, month, day
//*        returns 0 for success and 1 for error
//*
//***************************************************************
uint8_t RTC_Date(void) {
    int16_t setting[3]; //year, month, day, integer type is easier to bounds check
    uint8_t index;  //index into date, doubles as effectively a state as well
    char buf[5];
    uint16_t event;
    uint8_t done = false;
    
    DateTime now;
    
    now = rtc.now();

    setting[0] = now.year();
    setting[1] = (uint16_t) now.month();
    setting[2] = (uint16_t) now.day();
    

    GPIBInitTalk();
    GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end

    //set the menu line
    strcpy_P(data_buffer, PSTR("MESSAGE 1:\"                   UP    DOWN    SET\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
   
    index = 0;

    while (index < 3){
        //send message  to line 2 showing "^^" under first item of hms or ymd
        if (index ==0) strcpy_P(data_buffer, PSTR("MESSAGE 3:\"     SET YEAR:\""));
        else if (index == 1)strcpy_P(data_buffer, PSTR("MESSAGE 3:\"     SET MONTH:\""));
        else if (index == 2)strcpy_P(data_buffer, PSTR("MESSAGE 3:\"     SET DAY:\""));
        GPIBWriteData (data_buffer, strlen(data_buffer), true);

        done = false;
        while (!done) { 
            strcpy_P(data_buffer, PSTR("MESSAGE 2:\"     "));
            itoa(setting[index], buf, 10); //itoa() is inefficient, but simple to use.  Needs padding zero
            //pad month and day values
            if (index >0) strFix(buf); //pad zeros in the 2-digit values, ignore years
            strcat(data_buffer, buf);
            strcat_P(data_buffer, PSTR("\""));
//Serial.println(data_buffer);
            GPIBWriteData (data_buffer, strlen(data_buffer), true);
//Serial.println(F("finished write to screen"));      
            //wait for SRQ
//Serial.println(F("waiting for SRQ"));
            while (WaitForSRQ(0, 0xFFFE));  //returns a 1 if timeout occurs.  Keep polling SRQ until it is asserted (LOW): returns before timeout with a value of zero.
           //read event  
           event = GetEvent_SRQ();

           //then need to restore the controller pins to talk status and scope to listen status!!!
           GPIBInitTalk();  //don't know why this and the listen command need to be re-done....
           GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end
           
           //if event is up then increment setting[i], bounds check, wrap
           if (event == 452) { //UP
//            Serial.println(F("UP..."));
              setting[index]++;
              if (index == 0) {
                  if (setting[0] > 9999) setting[0] = 9999;
              }
              else if (index==1){
                  if (setting[1] > 12) setting[1] = 1;    
              }
              else if (index == 2) {
                  if (setting[2] > 31) setting[2] = 1;
              }
           }
           else if (event == 453) {//DOWN
              setting[index]--;
              if (index == 0) {
                  if (setting[0] <2000 ) setting[0] = 2000; //sanity check and to make setting random year easier
              }
              else if (index == 1){
                  if (setting[1] == 0) setting[1] = 12;    
              }
              else if (index == 2) {
                  if (setting[2] == 0) setting[2] = 31;
              }
           }

           else if (event == 454) {//set
              done = true;
              index++;  //move to next date value
           }
           
      
        } //end while !done...
      
//Serial.println(F("while ! done is complete"));
    } //end while index....
    
    strcpy_P(data_buffer,PSTR("MESSAGE 2:\"\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);


    //re-read the current DateTime to have fresh time data
    now = rtc.now();
    // set the new date, and keep most current time
    rtc.adjust(DateTime(setting[0], (uint8_t)setting[1], (uint8_t)setting[2], now.hour(), now.minute(),now.second()));

    //show new date
    now = rtc.now();
    strcpy_P(data_buffer,PSTR("MESSAGE 3:\"NEW DATE: "));
    itoa(now.month(), buf, 10);
    strFix(buf);
    //sprintf_P(buf, PSTR("%02d"), now.month());
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR("/"));
    itoa(now.day(), buf, 10);
    strFix(buf);

    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR("/"));
    itoa(now.year(),buf , 10);
    
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR("\""));  //add the non-displayed terminator
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    delay(4000);
    strcpy_P(data_buffer,PSTR("MESSAGE 3:\"\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
       
    return 0;  //no error detection at this time.
    
}




//***************************************************************
//*
//*    RTC_SetTime(void)
//*        Interface with scope to manage date and time
//*        sets the RTC hour, minute, second
//*        returns 0 for success and 1 for error
//*
//***************************************************************
uint8_t RTC_Time(void) {
    int8_t setting[3]; //hour, minute, second, integer type is easier to bounds check
    uint8_t index;  //index into date, doubles as effectively a state as well
    char buf[5];
    uint16_t event;
    uint8_t done = false;
    
    DateTime now;
    
    now = rtc.now();

    setting[0] = (uint16_t) now.hour();
    setting[1] = (uint16_t) now.minute();
    setting[2] = (uint16_t) now.second();
    

    GPIBInitTalk();
    GPIBWriteCmd(scopeL, strlen(scopeL), false);  //'true sets EOI at end

    //set the menu line
    strcpy_P(data_buffer, PSTR("MESSAGE 1:\"                   UP    DOWN    SET\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
   
    index = 0;

    while (index < 3){
        //send message  to line 2 showing "^^" under first item of hms or ymd
        if (index ==0) strcpy_P(data_buffer, PSTR("MESSAGE 3:\"     SET HOUR:\""));
        else if (index == 1)strcpy_P(data_buffer, PSTR("MESSAGE 3:\"     SET MINUTE:\""));
        else if (index == 2)strcpy_P(data_buffer, PSTR("MESSAGE 3:\"     SET SECOND:\""));
        GPIBWriteData (data_buffer, strlen(data_buffer), true);

        done = false;
        while (!done) { 
            strcpy_P(data_buffer, PSTR("MESSAGE 2:\"     "));
            itoa(setting[index], buf, 10); //itoa() is inefficient, but simple to use.  Needs padding zero
            strFix(buf);  //pad leading zeros in the values            
            strcat(data_buffer, buf);
            strcat_P(data_buffer, PSTR("\""));
//Serial.println(data_buffer);
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
//            Serial.println(F("UP..."));
              setting[index]++;
              if (index == 0) {
                  if (setting[0] > 23) setting[0] = 0;
              }
              else if (index==1){
                  if (setting[1] > 59) setting[1] = 0;    
              }
              else if (index == 2) {
                  if (setting[2] > 59) setting[2] = 0;
              }
           }
           else if (event == 453) {//DOWN
              setting[index]--;
              if (index == 0) {
                  if (setting[0] < 0) setting[0] = 23;
              }
              else if (index==1){
                  if (setting[1] < 0) setting[1] = 59;    
              }
              else if (index == 2) {
                  if (setting[2] < 0) setting[2] = 59;
              }
           }

           else if (event == 454) {//set
              done = true;
              index++;  //move to next time value
           }
           
      
        } //end while !done...
      
//Serial.println(F("while ! done is complete"));
    } //end while index....
    
    strcpy_P(data_buffer,PSTR("MESSAGE 2:\"\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);


    //re-read the current DateTime to have fresh time data
    now = rtc.now();
    // set the new date, and keep most current time
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), (uint8_t) setting[0], (uint8_t)setting[1], (uint8_t)setting[2]));

    //show new time
    now = rtc.now();  //load the date actually IN the RTC
    strcpy_P(data_buffer,PSTR("MESSAGE 3:\"NEW TIME: "));
    itoa(now.hour(), buf, 10);
    strFix(buf);
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR(":"));
    itoa(now.minute(), buf, 10);
    strFix(buf);
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR(":"));
    itoa(now.second(),buf , 10);
    strFix(buf);
    strcat(data_buffer,buf);
    strcat_P(data_buffer,PSTR("\""));  //add the non-displayed terminator
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    delay(4000);
    strcpy_P(data_buffer,PSTR("MESSAGE 3:\"\""));
    GPIBWriteData (data_buffer, strlen(data_buffer), true);
    


        
    return 0;  //no error detection at this time.

}
