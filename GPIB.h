#ifndef GPIB_h
#define GPIB_h

#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>

//to make porting of NI488.1 code easier here are some macros
#define bd 1;
#define ibwrt(hw,arr,len) GPIBWriteData(arr,len,true)  //write data to the bus, hw value is not used: this controller device is assumed
#define ibcmd(hw,arr,len) GPIBWriteCmd(arr, len, true) //Write a command to the bus, hw value is not used: this controller device is assumed
//#define ibrd(hw,arr,len) Read_Data()  //Puts data into standard data_buffer array, hw value is not used: this controller device is assumed
#define ibrd(hw,arr,len) GPIBGetData(arr, len, 10000)  //Puts data into standard data_buffer array, hw value is not used: this controller device is assumed

// PIN DEFINITIONS
#define _HIGH    1
#define _LOW     0
#define _ASSERTED  0
#define _RELEASED  1

//0=C4    4=C2
//1=c1    5=C1
//2=C    6=D3
//3=D4    7=D5

//NOTE:  THE UNO AND NANO HAVE ONLY 20 FREE I/O PINS:
//       THE SPI INTERFACE FOR THE SD CARD USES 4 PINS, AND THE H/W SERIAL PORT USED 2 MORE, LEAVING 14
//       THE GPIB BUS USES 16 PINS TOTAL: 8 DATA PINS, and DAV, NRFD, NDAC, EOI, ATN FOR 13 PINS ABSOLUTELY REQUIRED
//       THERE ARE ALSO REN, IFC, SRQ PINS. 
//
//       THIS SOFTWARE IS DESIGNED TO USE THE SRQ SIGNAL FOR WHEN THE SCOPE WANTS ATTENTION.  IT IS POSSIBLE TO
//       POLL THE SCOPE AND NOT USE THE SRQ SIGNAL AT ALL. BUT FOR THIS SOFTWARE, SRQ IS REQUIRED: 14 ESSENTIAL I/O PINS
//
//       THE IFC SIGNAL IS NICE TO HAVE SO THE SCOPE GPIB INTERFACE CAN BE CLEARED IF NECESSARY.  THIS DOES NOT SEEM TO BE VITAL.
//       THIS SOFTWARD DOES NOT USE THE REN SIGNAL.

//       SO THIS SOFTWARE, AS WRITTEN, USES A MINUMUM OF 14 I/O PINS FOR COMMUNICATING TO THE SCOPE AND 4 FOR COMMUNICATING TO
//       THE SD CARD: FOR A TOTAL OF 18 I/O PINS.  PINS WHICH HAVE EXTERNAL PULL UP/DOWN RESISTORS (LIKE THE LED PIN(S))
//       ARE NOT EASILY USED FOR GPIB OR SD FUNCTIONS EXCEPT FOR THE SD'S "CS" AND "SCK" SIGNALS. CS/SCK SIGNALS COULD TOLERATE
//       AN LED, MOST LIKELY.
//
//       I THINK IT MAY BE BARELY POSSIBLE TO USE A NANO FOR THIS PROJECT.  A MEGA IS DEFINITELY CAPABLE, WHICH IS WHAT I USE.


//  THESE UNO/NANO PIN DEFINITIONS ARE _N_O_T_ COMPATIBLE WITH USING AN SD CARD WITH FULL GPIB!
//  IT IS LIKELY THAT THE 13 ESSENTIAL GPIB SIGNALS, PLUS SRQ, AND THE 4 SPI SIGNALS CAN BE SUPPORTED ON THE NANO
//  BUT THESE DEFINITIONS ARE NOT GOING TO BE CORRECT!!!!!

#if defined (ARDUINO_AVR_UNO) || defined (ARDUINO_AVR_NANO)
#warning "UNO or Nano"
#error "NANO/UNO is not currently supported"
    //#Define DATA0 C.4
    #define DATA0_PORT PORTC
    #define DATA0_PIN PINC
    #define DATA0_DDR DDRC
    #define DATA0_BIT _BV(4)

    //#Define DATA1 C.3
    #define DATA1_PORT PORTC
    #define DATA1_PIN PINC
    #define DATA1_DDR DDRC
    #define DATA1_BIT _BV(3)

    //#Define DATA2 C.2
    #define DATA2_PORT PORTC
    #define DATA2_PIN PINC
    #define DATA2_DDR DDRC
    #define DATA2_BIT _BV(2)

    //#Define DATA3 C.1
    #define DATA3_PORT PORTC
    #define DATA3_PIN PINC
    #define DATA3_DDR DDRC
    #define DATA3_BIT _BV(1)

    //#Define DATA4 D.2
    #define DATA4_PORT PORTD
    #define DATA4_PIN PIND
    #define DATA4_DDR DDRD
    #define DATA4_BIT _BV(2)

    //#Define DATA5 D.3
    #define DATA5_PORT PORTD
    #define DATA5_PIN PIND
    #define DATA5_DDR DDRD
    #define DATA5_BIT _BV(3)

    //#Define DATA6 D.4
    #define DATA6_PORT PORTD
    #define DATA6_PIN PIND
    #define DATA6_DDR DDRD
    #define DATA6_BIT _BV(4)

    //#Define DATA7 D.5
    #define DATA7_PORT PORTD
    #define DATA7_PIN PIND
    #define DATA7_DDR DDRD
    #define DATA7_BIT _BV(5)
    
    //#Define REN   D.6
    #define REN_PORT PORTD
    #define REN_PIN PIND
    #define REN_DDR DDRD
    #define REN_BIT _BV(6)
    
    //#Define ATN   D.7
    #define ATN_PORT PORTD
    #define ATN_PIN PIND
    #define ATN_DDR DDRD
     #define ATN_BIT _BV(7)

    //#Define SRQ   B.0
    #define SRQ_PORT PORTB
    #define SRQ_PIN PINB
    #define SRQ_DDR DDRB
    #define SRQ_BIT _BV(0)
  
    //#Define IFC   B.1
    #define IFC_PORT PORTB
    #define IFC_PIN PINB
    #define IFC_DDR DDRB
    #define IFC_BIT _BV(1)

    //#Define NDAC  B.2
    #define NDAC_PORT PORTB
    #define NDAC_PIN PINB
    #define NDAC_DDR DDRB
    #define NDAC_BIT _BV(2)

    //#Define NRFD  B.3       ALSO USED BY SPI'S MISO SIGNAL
    #define NRFD_PORT PORTB
    #define NRFD_PIN PINB
    #define NRFD_DDR DDRB
    #define NRFD_BIT _BV(3)

    //#Define DAV   B.4       ALSO USED BY SPI'S MOSI SIGNAL
    #define DAV_PORT PORTB
    #define DAV_PIN PINB
    #define DAV_DDR DDRB
    #define DAV_BIT _BV(4)

    //#Define EOI   C.0
    #define EOI_PORT PORTC
    #define EOI_PIN PINC
    #define EOI_DDR DDRC
    #define EOI_BIT _BV(0)

    //#Define LED   B.5         ALSO USED BY SPI'S SCK SIGNAL
    #define LED_PORT PORTB
    #define LED_PIN PINB
    #define LED_DDR DDRB
    #define LED_BIT _BV(5)

    //#Define FREE  C.5
    #define CS_PIN 10   //using the SD shield, it uses pin 10 for the card select.


#elif defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560)  
     //#Define DATA0 A.0
    #define DATA0_PORT PORTA
    #define DATA0_PIN PINA
    #define DATA0_DDR DDRA
    #define DATA0_BIT _BV(0)

    //#Define DATA1 A.2
    #define DATA1_PORT PORTA
    #define DATA1_PIN PINA
    #define DATA1_DDR DDRA
    #define DATA1_BIT _BV(2)

    //#Define DATA2 A.4
    #define DATA2_PORT PORTA
    #define DATA2_PIN PINA
    #define DATA2_DDR DDRA
    #define DATA2_BIT _BV(4)

    //#Define DATA3 A.6
    #define DATA3_PORT PORTA
    #define DATA3_PIN PINA
    #define DATA3_DDR DDRA
    #define DATA3_BIT _BV(6)

    //#Define DATA4 A.1
    #define DATA4_PORT PORTA
    #define DATA4_PIN PINA
    #define DATA4_DDR DDRA
    #define DATA4_BIT _BV(1)

    //#Define DATA5 A.3
    #define DATA5_PORT PORTA
    #define DATA5_PIN PINA
    #define DATA5_DDR DDRA
    #define DATA5_BIT _BV(3)

    //#Define DATA6 A.5
    #define DATA6_PORT PORTA
    #define DATA6_PIN PINA
    #define DATA6_DDR DDRA
    #define DATA6_BIT _BV(5)

    //#Define DATA7 A.7
    #define DATA7_PORT PORTA
    #define DATA7_PIN PINA
    #define DATA7_DDR DDRA
    #define DATA7_BIT _BV(7)

    //#Define REN   C.6
    #define REN_PORT PORTC
    #define REN_PIN PINC
    #define REN_DDR DDRC
    #define REN_BIT _BV(6)
    
    //#Define ATN   C.1
    #define ATN_PORT PORTC
    #define ATN_PIN PINC
    #define ATN_DDR DDRC
     #define ATN_BIT _BV(1)

    //#Define SRQ   C.0
    #define SRQ_PORT PORTC
    #define SRQ_PIN PINC
    #define SRQ_DDR DDRC
    #define SRQ_BIT _BV(0)
  
    //#Define IFC   C.3
    #define IFC_PORT PORTC
    #define IFC_PIN PINC
    #define IFC_DDR DDRC
    #define IFC_BIT _BV(3)

    //#Define NDAC  C.2
    #define NDAC_PORT PORTC
    #define NDAC_PIN PINC
    #define NDAC_DDR DDRC
    #define NDAC_BIT _BV(2)

    //#Define NRFD  C.5
    #define NRFD_PORT PORTC
    #define NRFD_PIN PINC
    #define NRFD_DDR DDRC
    #define NRFD_BIT _BV(5)

    //#Define DAV   C.4
    #define DAV_PORT PORTC
    #define DAV_PIN PINC
    #define DAV_DDR DDRC
    #define DAV_BIT _BV(4)

    //#Define EOI   C.7
    #define EOI_PORT PORTC
    #define EOI_PIN PINC
    #define EOI_DDR DDRC
    #define EOI_BIT _BV(7)

    //#Define LED   B.7
    #define LED_PORT PORTB
    #define LED_PIN PINB
    #define LED_DDR DDRB
    #define LED_BIT _BV(7)

    //Define SwitchSense  L.4
    #define SENSE_PORT PORTL
    #define SENSE_PIN PINL
    #define SENSE_DDR DDRL
    #define SENSE_BIT _BV(4)

    //Define SwitchPoll1  L.3
    #define SPOLL1_PORT PORTL
    #define SPOLL1_PIN PINL
    #define SPOLL1_DDR DDRL
    #define SPOLL1_BIT _BV(3)

    //Define SwitchPoll2  L.2
    #define SPOLL2_PORT PORTL
    #define SPOLL2_PIN PINL
    #define SPOLL2_DDR DDRL
    #define SPOLL2_BIT _BV(2)

    //Define SwitchPoll3  L.1
    #define SPOLL3_PORT PORTL
    #define SPOLL3_PIN PINL
    #define SPOLL3_DDR DDRL
    #define SPOLL3_BIT _BV(1)

    //Define SwitchPoll4  L.0
    #define SPOLL4_PORT PORTL
    #define SPOLL4_PIN PINL
    #define SPOLL4_DDR DDRL
    #define SPOLL4_BIT _BV(0)

    //SD card defines
    //#define CS_PIN  53    //using the standard SPI pins on the Mega double row header, card select is pin 53
    #define CS_PIN  10    //using the Adafruit SD/RTC shield it uses pin 10.
    //#define MOSI_PIN  51  //PIN_SPI_MOSI
    //#define MISO_PIN 50   //PIN_SPI_MISO predefined value
    //#define SCK_PIN 52    //SCK_PIN is predefined!



#else
    #error "NO COMPATIBLE CPU DEFINED!!"
#endif



//#define DMASK 0x3C  //'0011-1100  'data high nibble shifted right 2
//#define CMASK 0x1e  //'0001-1110  'data low nibble shifted left 1





// SPECIAL CHARACTERS
#define _CR    13
#define _LF    10
#define _ESC   27
#define _PLUS  43





//GLOBAL DATA
//uint8_t receiveState = 0;     //for state machine of receive GPIB data
//bool receivingData  = false;   //indicates GPIB receive is in progress




//Dim CFlip as ByteVectorData ({
//  &H00, &H08, &H04, &H0C
//  &H02, &H0A, &H06, &H0E
//  &H01, &H09, &H05, &H0D
//  &H03, &H0B, &H07, &H0F
//})

//This will look up the reverse bits in a nibble
//uint8_t bitReverse[] ={0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09, 0x05, 0x0D, 0x03, 0x0B, 0x07, 0x0F};






// PUBLIC USER INTERFACE FUNCTIONS
void        GPIBInit(void);
uint16_t    GPIBWriteData(char* data, uint16_t count, bool useEOI);
uint16_t    GPIBWriteString (char* data, bool useEOI);
void        GPIBWriteCmd(char* data, uint16_t _length, bool useEOI);
void        GPIBInitListen(void);
void        GPIBInitTalk();
void        GPIBInterfaceClear(void);
uint8_t     GPIBDataReady(void);
uint16_t    GPIBGetData(char* buffer, uint16_t len, uint16_t timeOut = 10000);
void        GPIBSendState(void);

//  PRIVATE FUNCTIONS
void    GPIBRelease(volatile uint8_t *gpibPORT, volatile uint8_t *gpibDDR, uint8_t gpibBIT);
void    GPIBAssert(volatile uint8_t *gpibPORT, volatile uint8_t *gpibDDR, uint8_t gpibBIT);
void    GPIBClear(volatile uint8_t *gpibPORT, volatile uint8_t *gpibDDR, uint8_t gpibBIT);
void    GPIBClearData(void);
void    GPIBClearData_Talk(void);
int     GPIBCheck(void);


void    GPIBRemoteEnable(bool state);

uint8_t WaitForDAV(uint8_t state, uint16_t timeLimit);
uint8_t WaitForNRFD(uint8_t state, uint16_t timeLimit);
uint8_t WaitForNDAC(uint8_t state, uint16_t timeLimit);
uint8_t WaitForSRQ(uint8_t state, uint16_t timeLimit);

uint8_t GPIBReadByte(void);
void    _WriteByte(uint8_t data);
uint8_t GPIBWriteByte(uint8_t datum, bool useEOI);


#endif
