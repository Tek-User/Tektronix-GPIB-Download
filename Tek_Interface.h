#ifndef tek_interface_h
#define tek_interface_h


#define USE_SRQ 1
#define DEBUG 1


#define ClientADDR 1  //GPIB address of the client
#define LLO 0x11
#define DCL 0x14
#define SPE_EN 0x18  //can't use "SPE" because that is already used by the Arduino system
#define SPD 0x19
#define UNL 0x3F //=63d
#define UNT 0x5F 
#define TADDR 64
#define LADDR 32


//define the channels which will be checked and read.
#define CH1 0
#define CH2 1
#define ADD 2
#define MULT 3
#define REF1  4
#define REF2  5
#define REF3  6
#define REF4  7
#define NUMCHANNELS 8
//define more channels here as needed

#define NUMPOINTS 1024    //the number of data points in the scope trace


void    tek_Setup(void);
void    tek_InitScope(void);
void    displayedChannels(uint8_t *list);
uint8_t CheckChannel(uint8_t channel);
void    ReadWfmPre(uint8_t channel);
void    ReadCurve(uint8_t channel);
uint8_t ParseWFM(void);
void    ParseData(void);
uint16_t Read_Data(void);
uint16_t GetEvent_SRQ(void);
uint16_t GetEvent(void);
void    introScreen(void);
void strFix(char * buf);
uint8_t checkID(void);
void    tek_Message(const char *mess);
void    tek_RingBell(void);
void    tek_Menu(uint8_t state, uint8_t force = 0);
void    tek_SetFileName(void);
//uint16_t my_atoi(const char *str);








#endif
