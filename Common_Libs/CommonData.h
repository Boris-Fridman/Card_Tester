#ifndef ____CommonData_h__
#define ____CommonData_h__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 * Terminal Colors
 *
 * The information about colors can be found in the sine:
 * https://en.wikipedia.org/wiki/ANSI_escape_code
 */
#define TermBlack         "\033[20m"
#define TermDarkRed       "\033[21m"
#define TermDarkGreen     "\033[22m"
#define TermDarkYello     "\033[23m"
#define TermDarkBlue      "\033[24m"
#define TermDarkMagenta   "\033[25m"
#define TermDarkCyan      "\033[26m"
#define TermGrayL         "\033[27m"    //  "Dark White"   (Gray 90  According to site https://github.com/ThomasDickey/xterm-snapshots/blob/master/XTerm-col.ad)

#define TermGrayD         "\033[30m"    //  "Light Black"  (Gray 50  According to site https://github.com/ThomasDickey/xterm-snapshots/blob/master/XTerm-col.ad)
#define TermRed           "\033[31m"
#define TermGreen         "\033[32m"
#define TermYello         "\033[33m"
#define TermBlue          "\033[34m"
#define TermMagenta       "\033[35m"
#define TermCyan          "\033[36m"
#define TermWhite         "\033[37m"

#define TermColorsReset   "\033[39;49m"





/*
 * Min / Max macros
 */

#define MAX(X, Y) ( (X) > (Y) ? (X) : (Y) )           /* The macro returning the biggest from the two values.  */
#define MIN(X, Y) ( (X) < (Y) ? (X) : (Y) )           /* The macro returning the smallest from the two values. */

/*
 * Div macros
 */

#define DIV_RND(X,Y)    ( ((X) + (Y) / 2) / (Y) )     /* Deviation with rounding.      10/6 will give 2 and 10/3 will give 3  */
#define DIV_RND_UP(X,Y) ( ((X) + (Y) - 1) / (Y) )     /* Deviation with rounding up.   10/6 will give 2 and 10/3 will give 4  */
#define DIV_RND_DN(X,Y) ( (X) / (Y)             )     /* Deviation with rounding down. 10/6 will give 1 and 10/3 will give 3 Regular deviation equal to regular "/". Is defined for compliation the previous macros deviations */


/*
 * Test data definitions
 */

#define MAX_POSSIBLE_FREAUENCY  1000000000  /* Hz */      /* Maximal possible frequency used for calculation maximal timeout */
#define MIN_I2C_FREQUENCY       100000      /* Hz */
#define MIN_SPI_FREQUENCY       200000      /* Hz */
#define MIN_UART_FREQUENCY      50000       /* Hz */

#define MAX_TEST_PATTERN_SIZE   100                       /* Maximum permitted Length of pattern. */
#define MIN_TIME_OUT            1000        /* ms  */     /* Minimal timeout to wait for response. */

#define MAX_ADC_CHECK_TIME      1           /* ms  */     /* Maximal time period required for testing ADC in one interration.*/


/*
 * Internet information for sending and receiving call-events.
 */
#define HOST_NAME              "CardTester"
#define DESTIN_IP              "192.168.1.113"         /* Server IP address to which are sent the call messages. */
#define DESTIN_PORT             8080                   /* Server port to which are sent the call messages. */
#define BUFFER_SIZE             1024                   /* The length in bytes, of the buffer pointed by the buf paramter that is used by the recvfrom() function. */

#define CRC_SIZE               ( sizeof(uint32_t) )
#define MAX_SEND_MSG_SIZE      ( sizeof(TestData_s) + MAX_TEST_PATTERN_SIZE + CRC_SIZE )
#define MIN_SEND_MSG_SIZE      ( sizeof(TestData_s) + CRC_SIZE )
#define MAX_RECV_MSG_SIZE      ( sizeof(TestResult_s) + CRC_SIZE )
#define MIN_RECV_MSG_SIZE      ( sizeof(TestResult_s) + CRC_SIZE )


typedef enum __attribute__((__packed__)) PeriphType_e  // The attribute "__attribute__((__packed__))" is defined to make the enumeration to be in one byte to ensure the correct data length while sending. 
 {
  E_TIMER,
  E_UART,
  E_SPI,
  E_I2C,
  E_ADC,

  E_NUM_PERIPHS
 }PeriphType_e;

typedef enum __attribute__((__packed__)) TestResType_e  // The attribute "__attribute__((__packed__))" is defined to make the enumeration to be in one byte to ensure the correct data length while sending. 
 {
  E_TEST_SUCCEEDED =    0,
  E_TEST_FAILED    = 0xFF
 }TestResType_e;


typedef enum VoltsConvMethod_e  // Conversion method between voltage and rough data.
 {
  E_IDEAL_CONV,    // Method of ideal conversion when the ADC or DAC are work ideally and the data can be converted by the theoretical formula.
  E_LINEAR_CONV,   // Conversion by using the linear equation y=ax+b when the coefficients a and b are found by linear regression or any other method for getting an approximate linear equation.
  E_POLYNOM_CONV   // Conversion by the tailor polynomial approximation y= ∑aₙ𐄁xⁿ = a₀+ a₁x+a₂x²+a₃x³+a₄x⁴+a₅x⁵+a₆x⁶+a₇x⁷+a₈x⁸+a₉x⁹+...  .
 }VoltsConvMethod_e;


typedef struct PeriphBitField_s
 {
  uint8_t Timer_bf     : 1;
  uint8_t UART_bf      : 1;
  uint8_t SPI_bf       : 1;
  uint8_t I2C_bf       : 1;
  uint8_t ADC_bf       : 1;
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
  PeriphBitField_s Results_B_F;
  TestResType_e TestResult;
 }TestResult_s;




#define Timer_Flag (1 << E_TIMER)
#define UART_Flag  (1 << E_UART)
#define SPI_Flag   (1 << E_SPI)
#define I2C_Flag   (1 << E_I2C)
#define ADC_Flag   (1 << E_ADC)


 extern char const * const ResultColors[];
 extern char const * const ResultMessages[];

 extern char const * const PeriphNames[E_NUM_PERIPHS];


#define DEF_INIT_VAL 0xEF45AB12

uint32_t FindCRC(uint8_t * Data, uint8_t Length, uint32_t InitVal);

void Add_CRC(uint8_t buf[], size_t len);
bool CRC_Correct(uint8_t buf[], size_t len);




#endif  //  ____CommonData_h__

