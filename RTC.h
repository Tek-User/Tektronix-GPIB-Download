#ifndef RTC_h
#define RTC_h

#include "RTClib.h"

//*** PUBLIC FUNCTIONS
void RTC_setup(void);
void  RTC_GetRTCDate(uint16_t &year, uint8_t &month, uint8_t &day);
void  RTC_SetRTCDate(uint16_t &year, uint8_t &month, uint8_t &day);
uint8_t RTC_Date(void);
uint8_t RTC_Time(void);


#endif
