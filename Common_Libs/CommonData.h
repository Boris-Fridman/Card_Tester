#ifndef ____CommonData_h__
#define ____CommonData_h__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX(X, Y) ( (X) > (Y) ? (X) : (Y) )           /* The macro returning the biggest from the two values.  */
#define MIN(X, Y) ( (X) < (Y) ? (X) : (Y) )           /* The macro returning the smallest from the two values. */



#define MAX_TEST_PATTERN_SIZE   100                       /* Maximum permitted Length of pattern. */
#define MIN_TIME_OUT            1000        /* ms  */     /* Minimal timeout to wait for response. */

#define MAX_POSSIBLE_FREAUENCY  1000000000  /* Hz */      /* Maximal possible frequency used for calculation maximal timeout */
#define MIN_I2C_FREQUENCY       100000      /* Hz */
#define MIN_SPI_FREQUENCY       200000      /* Hz */
#define MIN_UART_FREQUENCY      50000       /* Hz */

/* Internet information for sending and receiving call-events. */
#define HOST_NAME    "CardTester"
#define DESTIN_IP    "192.168.1.113"         /* Server IP address to which are sent the call messages. */
#define DESTIN_PORT   8080                   /* Server port to which are sent the call messages. */
#define BUFFER_SIZE   1024                   /* The length in bytes, of the buffer pointed by the buf paramter that is used by the recvfrom() function. */





typedef enum __attribute__((__packed__)) PeriphType_e  // The attribute "__attribute__((__packed__))" is defined to make the enumeration to be in one byte to ensure the correct data length while sending. 
 {
  E_TIMER =  1,
  E_UART  =  2,
  E_SPI   =  4,
  E_I2C   =  8,
  E_ADC   = 16
 }PeriphType_e;

typedef enum __attribute__((__packed__)) TestResType_e  // The attribute "__attribute__((__packed__))" is defined to make the enumeration to be in one byte to ensure the correct data length while sending. 
 {
  E_TEST_SUCCEEDED =    0,
  E_TEST_FAILED    = 0xFF
 }TestResType_e;


typedef struct PeriphBitField_s
 {
  uint8_t Timer_Flag     : 1;
  uint8_t UART_Flag      : 1;
  uint8_t SPI_Flag       : 1;
  uint8_t I2C_Flag       : 1;
  uint8_t ADC_Flag       : 1;
  uint8_t Reserved  : 3;
 }PeriphBitField_s;

 typedef struct __attribute__((packed)) TestData_s // The attribute "__attribute__((packed))" is defined to make the struct to be at the exact size as it is defined to ensure the correct data lendght while sending.
 {
  uint32_t Test_ID;
  uint32_t TestVoltage;
  uint32_t TestTime;
  PeriphBitField_s Periph_B_F;
  uint8_t Num_Interations;
  uint8_t Bit_Pattern_Length;
 }TestData_s;


typedef struct __attribute__((packed)) TestResult_s // The attribute "__attribute__((packed))" is defined to make the struct to be at the exact size as it is defined to ensure the correct data lendght while sending.
 {
  uint32_t Test_ID;
  PeriphBitField_s Periph_B_F;
  TestResType_e TestResult;
 }TestResult_s;


#define DEF_INIT_VAL 0xEF45AB12

uint32_t FindCRC(uint8_t * Data, uint8_t Length, uint32_t InitVal);


#endif  //  ____CommonData_h__

