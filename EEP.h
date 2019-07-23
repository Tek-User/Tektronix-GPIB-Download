#ifndef EEP_h
#define EEP_h



void    EEP_Init(uint8_t force = 0);
uint8_t EEP_ReadNameStub(char* dest);
uint8_t EEP_WriteNameStub(char* buf);

#endif
