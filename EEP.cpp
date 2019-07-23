#include "Arduino.h"
#include "EEP.h"
#include <avr/eeprom.h>
#include <string.h>


uint8_t EEMEM EEfirstTime;
uint8_t EEMEM EEnameStub[8];  //8 charcters total, 3 are the numeric series, leaving 5+null+2spare = 8




void EEP_Init(uint8_t force)
{
    char newNameStub[] = "TEK";

    if(force)
    {
        eeprom_update_block((void*)&newNameStub, (void *)&EEnameStub, strlen(newNameStub)+1);
        eeprom_update_byte(&EEfirstTime,255);
        Serial.println("EEPROM FORCED TO INITIALIZE");
    }

    
    else if (eeprom_read_byte(&EEfirstTime) != 1)
    {
         eeprom_update_block((void*)&newNameStub, (void *)&EEnameStub, strlen(newNameStub)+1);
         eeprom_update_byte(&EEfirstTime,1);
         Serial.println("EEPROM INITIALIZED");
    }
}



//**************************************************************************
//*
//*     Copies up to 7 bytes from the name stub into the destination in RAM
//*
//*     Returns the number of bytes copied including the terminal null
//*     Anything longer than 6 total is illegal for the scope interface progam
//*
//**************************************************************************

uint8_t EEP_ReadNameStub(char* dest)
{
       //read byte by byte the name string until a null is encountered
       //max 7 characters: should be no more than 5 plus null

       uint8_t num=0;
  
       while (num < 8)
       {
             dest[num] = eeprom_read_byte(&EEnameStub[num]);
             //Serial.print(num);
             //Serial.print("/");
             //Serial.print((uint16_t)&EEnameStub[num]);
             //Serial.print("/");
             //Serial.println(dest[num]);
             if (dest[num] == 0)break;
             num++;
       }
       return num+1;
}

    



//**************************************************************************
//*
//*     Copies up to 7 bytes from RAM to the EEPROM name stub
//*
//*     Returns the number of bytes copied including the terminal null
//*     Anything longer than 6 total is illegal for the scope interface progam
//*
//**************************************************************************
uint8_t EEP_WriteNameStub(char* buf)
{
      //xxx
      uint8_t num=0;
      char newNameStub[8];
      
      while (num < 8)
      {
            eeprom_update_byte(&EEnameStub[num], buf[num]);
            Serial.print(num);
            Serial.print("/");
            Serial.print((uint16_t)&EEnameStub[num]);
            Serial.print("/");
            Serial.println(buf[num]);
            if (buf[num] == 0)break;
            num++;
      }

    newNameStub[0] = 0; //zero-out the name
    eeprom_read_block((void*)&newNameStub, (void *)&EEnameStub, 6);

    Serial.print("The new EEPROM copied back reads >");
    Serial.print(newNameStub);
    Serial.println("<");

      return num+1;
}
