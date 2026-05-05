#ifndef ____CommonData_h__
#define ____CommonData_h__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
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





/**
 * Min / Max macros
 */
#define MAX(X, Y) ( (X) > (Y) ? (X) : (Y) )           /* The macro returning the biggest from the two values.  */
#define MIN(X, Y) ( (X) < (Y) ? (X) : (Y) )           /* The macro returning the smallest from the two values. */

/**
 * Div macros
 */
#define DIV_RND(X,Y)    ( ((X) + (Y) / 2) / (Y) )     /* Deviation with rounding.      10/6 will give 2 and 10/3 will give 3  */
#define DIV_RND_UP(X,Y) ( ((X) + (Y) - 1) / (Y) )     /* Deviation with rounding up.   10/6 will give 2 and 10/3 will give 4  */
#define DIV_RND_DN(X,Y) ( (X) / (Y)             )     /* Deviation with rounding down. 10/6 will give 1 and 10/3 will give 3 Regular deviation equal to regular "/". Is defined for compliation the previous macros deviations */

/**
 * Macro for preventing warnings in case of unused variables.
 */
#if !defined(UNUSED)
#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */
#endif /* UNUSED */



/**
 * Test data definitions
 */

#define MAX_POSSIBLE_FREAUENCY  1000000000  /* Hz */      /* Maximal possible frequency used for calculation maximal timeout */
#define MIN_I2C_FREQUENCY       100000      /* Hz */
#define MIN_SPI_FREQUENCY       200000      /* Hz */
#define MIN_UART_FREQUENCY      50          /* Hz */

#define MAX_TEST_PATTERN_SIZE   200                       /* Maximum permitted Length of pattern. Also is used as the maximal size of code segment sent during OTA Update of the Test Program . */
#define MIN_TIME_OUT            1000        /* ms  */     /* Minimal timeout to wait for response. */

#define MAX_ADC_CHECK_TIME      50          /* µs  */     /* Maximal time period required for testing ADC in one interration is given in micorseconds.*/

/**
 * Internet information for sending and receiving call-events.
 */
#define HOST_NAME              "CardTester"               /* Server Host Name with which the DHCP Connection is recognized. */
#define BL_HOST_NAME           "BL_of_CardTester"        /* Server Host Name for BootLoader with which the DHCP Connection is recognized. */
#define DESTIN_IP              "192.168.1.113"            /* Server IP address to which are sent the call messages. */
#define DESTIN_PORT             8080                      /* Server port to which are sent the call messages. */
#define BUFFER_SIZE             1024                      /* The length in bytes, of the buffer pointed by the buf paramter that is used by the recvfrom() function. */

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


typedef struct PeriphBitField_s
 {
  uint8_t Timer_bf       : 1;
  uint8_t UART_bf        : 1;
  uint8_t SPI_bf         : 1;
  uint8_t I2C_bf         : 1;
  uint8_t ADC_bf         : 1;
  uint8_t OTA_UPDATE_bf  : 1;
  uint8_t Reserved       : 2;
 }PeriphBitField_s;

 typedef struct __attribute__((packed)) TestData_s /* The attribute "__attribute__((packed))" is defined to make the struct to be at the exact size as it is defined to ensure the correct data lendght while sending. */
 {
  uint32_t Test_ID;
  uint32_t TestTime;
  int32_t TestVoltage;
  PeriphBitField_s Periph_B_F;
  uint8_t Num_Interations;
  uint8_t Bit_Pattern_Length;
 }TestData_s;


typedef struct __attribute__((packed)) TestResult_s /* The attribute "__attribute__((packed))" is defined to make the struct to be at the exact size as it is defined to ensure the correct data lendght while sending. */
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


extern char const * const ResultColors[];   /* Colors for showing results : [0] - for fail result and [1] - for pass result. */
extern char const * const ResultMessages[]; /* Messages for showing result: [0] - for fail result and [1] - for pass result. */

extern char const * const PeriphNames[E_NUM_PERIPHS]; /* Names for peripherals that the card can test. */

extern TestData_s const ResetCondition;  /* Is used for making reset to the board card. */

#define DEF_INIT_VAL 0xEF45AB12

/**
 *
 */
uint32_t FindCRC(uint8_t * Data, uint8_t Length, uint32_t InitVal);

/**
 *
 */
void Add_CRC(uint8_t buf[], size_t len);

/**
 *
 */
bool CRC_Correct(uint8_t buf[], size_t len);

/**
 * @brief
 * This function decodes the packet received from network.
 * Attention !!!
 * The last parameter "TestPattern" is given as pointer to pointer to dynamically allocated memory.
 * That means that at the end of the program it must be freed by the procedure "FreeTestPattern()" to avoid the memory leakage.
 *
 * @code
 * void DecodeReqData(uint8_t Data[], size_t Len, TestData_s *TestData, uint8_t **TestPattern);
 * @code
 *
 * @param Data         The pointer to data existing in the packet.
 *
 * @param Len          The length of the data.
 *
 * @param TestData     The The data extracted from the packet. given as the structure "TestData_s".
 *
 * @param TestPattern  The pattern for testing the data. Is given as the pointer to pointer to start of the data.
 *                     Must be freed at the end of usage by the procedure "FreeTestPattern()" to prevent the memory leakage.
 *
 *
 */
void DecodeReqData(uint8_t Data[], size_t Len, TestData_s *TestData, uint8_t **TestPattern);

/**
 * @brief
 * This Procedure is used for freeing the "**TestPattern" reserved by the procedure "DecodeReqData()".
 * No need to check anything before running it because it checks automatically inside if the memory
 * is reserved and sets the pointer to NULL after freeing it.
 *
 * @code
 * FreeTestPattern(uint8_t **TestPattern);
 * @code
 *
 * @param TestPattern   The pointer to data reserved by the "DecodeReqData()" procedure that must be freed.
 *                      After freeing it will be set to NULL.
 *
 */
void FreeTestPattern(uint8_t **TestPattern);

/**
 * @brief
 * Encodes data for response to the parameter "**RespData".
 * Attention !!!
 * The procedure allocates dynamic memory for to which the parameter "**RespDtata" points.
 * The freeing must be done by the "FreeRespData()".
 *
 * @code
 * size_t EncodeRespData(TestResult_s *TestResult, uint8_t **RespData);
 * @code
 *
 * @param TestResult   Contains the Test Result answer that must be encoded for giving the response.
 *
 * @param RespData     The encoded data for sending via the network. Must be freed after usage by the procedure "FreeRespData()".
 *
 */
size_t EncodeRespData(TestResult_s *TestResult, uint8_t **RespData);

/**
 * @brief Frees the "**RespData" allocated by the procedure "EncodeRespData()".
 * No need to check the condition. It checks if the pointer is not NULL and only in this case it frees the memory.
 * After freeng the memory it sets the pointer to NULL.
 *
 * @code
 * void FreeRespData(uint8_t **RespData);
 * @code
 *
 * @param RespData  The pointer to pointer to the dynamically allocated memory for freeing. *
 *
 */
void FreeRespData(uint8_t **RespData);



#define START_FLASH_ADDRESS   0x08000000
#define START_PROG_OFFSET     0x18000  // 0x00000    0x08000 0x10000 0x18000   0x20000  0x40000   0x80000   0xC0000
#define START_PROG_ADDRESS    (START_FLASH_ADDRESS + START_PROG_OFFSET)

#endif  //  ____CommonData_h__

