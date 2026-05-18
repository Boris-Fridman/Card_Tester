#ifndef ____CommonData_h__
#define ____CommonData_h__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>

/*======================================================================================================================*/

/**
 * Terminal Colors
 *
 * The information about colors can be found in the sine:
 * https://en.wikipedia.org/wiki/ANSI_escape_code
 */
/*----------------------------------------------------------------------------------------------------------------------*/
/*  Foreground Colors                                                                                                   */
#define TermBlack           "\033[30m"
#define TermRed             "\033[31m"
#define TermGreen           "\033[32m"
#define TermYello           "\033[33m"
#define TermBlue            "\033[34m"
#define TermMagenta         "\033[35m"
#define TermCyan            "\033[36m"
#define TermWhite           "\033[37m"  /* (Light Gray) */

#define TermBrightBlack     "\033[90m"  /* (Dark Gray)  */
#define TermBrightRed       "\033[91m"
#define TermBrightGreen     "\033[92m"
#define TermBrightYello     "\033[93m"
#define TermBrightBlue      "\033[94m"
#define TermBrightMagenta   "\033[95m"
#define TermBrightCyan      "\033[96m"
#define TermBrightWhite     "\033[97m"

/*----------------------------------------------------------------------------------------------------------------------*/
/*  Background Colors                                                                                                   */
#define TermBGBlack         "\033[40m"
#define TermBGRed           "\033[41m"
#define TermBGGreen         "\033[42m"
#define TermBGYello         "\033[43m"
#define TermBGBlue          "\033[44m"
#define TermBGMagenta       "\033[45m"
#define TermBGCyan          "\033[46m"
#define TermBGWhite         "\033[47m"   /* (Light Gray) */

#define TermBGBrightBlack   "\033[100m"  /* (Dark Gray)  */
#define TermBGBrightRed     "\033[101m"
#define TermBGBrightGreen   "\033[102m"
#define TermBGBrightYello   "\033[103m"
#define TermBGBrightBlue    "\033[104m"
#define TermBGBrightMagenta "\033[105m"
#define TermBGBrightCyan    "\033[106m"
#define TermBGBrightWhite   "\033[107m"

#define TermColorsReset   "\033[39;49m"



/*======================================================================================================================*/

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
#define DIV_RND_DN(X,Y) ( (X) / (Y)             )     /* Deviation with rounding down. 10/6 will give 1 and 10/3 will give 3 Regular deviation equal to regular "/". Is defined for compilation the previous macros deviations */

/*======================================================================================================================*/

/**
 * Macro for preventing warnings in case of unused variables.
 */
#if !defined(UNUSED)
#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */
#endif /* UNUSED */

/*======================================================================================================================*/

/**
 * Test data definitions
 */
#define MAX_POSSIBLE_FREAUENCY  1000000000  /* Hz */      /* Maximal possible frequency used for calculation maximal timeout */
#define MIN_I2C_FREQUENCY       100000      /* Hz */
#define MIN_SPI_FREQUENCY       200000      /* Hz */
#define MIN_UART_FREQUENCY      50          /* Hz */

#define MAX_TEST_PATTERN_SIZE   200                       /* Maximum permitted Length of pattern. Also is used as the maximal size of code segment sent during OTA Update of the Test Program . */
#define MIN_TIME_OUT            1000        /* ms  */     /* Minimal timeout to wait for response. */

#define MAX_ADC_CHECK_TIME      50          /* µs  */     /* Maximal time period required for testing ADC in one interaction is given in microseconds.*/

/*======================================================================================================================*/

/**
 * Internet information for sending and receiving call-events.
 */
#define HOST_NAME              "CardTester"               /* Server Host Name with which the DHCP Connection is recognized. */
#define BL_HOST_NAME           "BL_of_CardTester"         /* Server Host Name for BootLoader with which the DHCP Connection is recognized. */
#define DESTIN_IP              "192.168.1.113"            /* Server IP address to which are sent the call messages. */
#define DESTIN_PORT             8080                      /* Server port to which are sent the call messages. */
#define BUFFER_SIZE             1024                      /* The length in bytes, of the buffer pointed by the buf parameter that is used by the recvfrom() function. */

#define CRC_SIZE               ( sizeof(uint32_t) )
#define MAX_SEND_MSG_SIZE      ( sizeof(TestData_s) + MAX_TEST_PATTERN_SIZE + CRC_SIZE )
#define MIN_SEND_MSG_SIZE      ( sizeof(TestData_s) + CRC_SIZE )
#define MAX_RECV_MSG_SIZE      ( sizeof(TestResult_s) + CRC_SIZE )
#define MIN_RECV_MSG_SIZE      ( sizeof(TestResult_s) + CRC_SIZE )


/*======================================================================================================================*/

#define START_FLASH_ADDRESS   0x08000000
#define START_PROG_OFFSET     0x20000  // 0x00000    0x08000 0x10000 0x18000   0x20000  0x40000   0x80000   0xC0000
#define START_PROG_ADDRESS    (START_FLASH_ADDRESS + START_PROG_OFFSET)

/*======================================================================================================================*/


typedef enum CommandTypes_e
 {
  OTA_START = 0x01,
  OTA_DATA  = 0x02,
  OTA_END   = 0x03
 }
CommandTypes_e;


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

 typedef struct __attribute__((packed)) TestData_s /* The attribute "__attribute__((packed))" is defined to make the struct to be at the exact size as it is defined to ensure the correct data length while sending. */
 {
  uint32_t Test_ID;               /* In case of OTA Update is used as start address in flash of code segment. */
  uint32_t TestTime;
  int32_t TestVoltage;
  PeriphBitField_s Periph_B_F;
  uint8_t Num_Interations;        /* In case of OTA Update is used as Command Type.  */
  uint8_t Bit_Pattern_Length;     /* In case of OTA Update is used as code segment length. */
 }TestData_s;


typedef struct __attribute__((packed)) TestResult_s /* The attribute "__attribute__((packed))" is defined to make the struct to be at the exact size as it is defined to ensure the correct data length while sending. */
 {
  uint32_t Test_ID;              /* In case of OTA Update is used as start address in flash of code segment. */
  PeriphBitField_s Periph_B_F;
  PeriphBitField_s Results_B_F;
  TestResType_e TestResult;      /* In case of OTA Update is used for signalizing if the burning was success of failed. */
 }TestResult_s;

typedef enum LnPrt_e
 {
  E_CURS_TO_END,
  E_CURS_TO_BEG,
  E_FULL_LINE
 }LnPrt_e;

/*======================================================================================================================*/

#define Timer_Flag (1 << E_TIMER)
#define UART_Flag  (1 << E_UART)
#define SPI_Flag   (1 << E_SPI)
#define I2C_Flag   (1 << E_I2C)
#define ADC_Flag   (1 << E_ADC)


/*======================================================================================================================*/

extern char const * const ResultColors[];   /* Colors for showing results : [0] - for fail result and [1] - for pass result. */
extern char const * const ResultMessages[]; /* Messages for showing result: [0] - for fail result and [1] - for pass result. */

extern char const * const PeriphNames[E_NUM_PERIPHS]; /* Names for peripherals that the card can test. */

extern TestData_s const ResetCondition;  /* Is used for making reset to the board card. */

/*======================================================================================================================*/

/**
 * @brief Calculates CRC from given block of data.
 *
 * @code
 * uint32_t FindCRC(uint8_t * Data, uint8_t Length, uint32_t InitVal);
 * @code
 *
 * @param Data     The pointer to the first address of memory in which the data exists.
 *
 * @param Length   The length of the data.
 *
 * @param InitVal The initialization value.
 *
 * @return Calculated CRC.
 *
 */
uint32_t FindCRC(uint8_t * Data, uint8_t Length, uint32_t InitVal);

/**
 * @brief Appends to the end of the data array the calculated CRC from it. The length must include the place of the CRC.
 *        For example if the data has length 8 the length given as parameter must be 12 = 8 + 4. The CRC has 4 bytes of length.
 *
 * @code
 * void Add_CRC(uint8_t buf[], size_t len);
 * @code
 *
 * @param buf  The start of data.
 *
 * @param len  The length of data including CRC size. For example if buffer has length of 8 bytes the len must be 12: 8 +4 = 12.
 */
void Add_CRC(uint8_t buf[], size_t len);

/**
 * @brief Checks if the CRC is correct.
 *
 * @code
 * bool CRC_Correct(uint8_t buf[], size_t len);
 * @code
 *
 * @param buf   The start of data
 *
 * @param len   The length of data. Including CRC.
 *              For example if the given length is 12 the CRC checking will be made from the first 8 bytes
 *              and the result will be compared to the last 4 bytes.
 *
 */
bool CRC_Correct(uint8_t buf[], size_t len);

/*======================================================================================================================*/


/**
 * @brief
 * This function encodes the packet received from network.                                     
 * Attention !!!                                                                               
 * The last parameter "ReqData" is given as pointer to pointer to dynamically allocated memory.
 * That means that at the end of the program it must be freed by the procedure "FreeData()"    
 * to avoid the memory leakage.                                                                
 *
 * @code
 * ssize_t EncodeReqData(TestData_s const * const TestData, uint8_t TestPattern[], uint8_t **ReqData);
 * @code
 *
 * @param TestData     The pointer in memory pattern for the data for testing.
 *
 * @param TestPattern  The pointer to the test pattern that must be added to the data of NULL if the pattern doesn't exist.
 *
 * @param ReqData      The pointer to pointer to the encoded data that will be set to the new reserved memory to which the data will be copied.
 *                     Must be freed at the end of usage by the procedure "FreeData()" to prevent the memory leakage.
 *
 * @return             The length of data if encoded successfully or "0" if failed.
 */
ssize_t EncodeReqData(TestData_s const * const TestData, uint8_t TestPattern[], uint8_t **ReqData);

/**
 * @brief
 * This function decodes the packet received from network.
 * Attention !!!
 * The last parameter "TestPattern" is given as pointer to pointer to dynamically allocated memory.
 * That means that at the end of the program it must be freed by the procedure "FreeData()" to avoid the memory leakage.
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
 *                     Must be freed at the end of usage by the procedure "FreeData()" to prevent the memory leakage.
 * 
 * @return "true" if the data was decoded successfully. "false" otherwise.
 */
bool DecodeReqData(uint8_t Data[], size_t Len, TestData_s *TestData, uint8_t **TestPattern);


/**
 * @brief
 * Encodes data for response to the parameter "**RespData".
 * Attention !!!
 * The procedure allocates dynamic memory for to which the parameter "**RespDtata" points.
 * The freeing must be done by the "FreeData()".
 *
 * @code
 * size_t EncodeRespData(TestResult_s *TestResult, uint8_t **RespData);
 * @code
 *
 * @param TestResult   Contains the Test Result answer that must be encoded for giving the response.
 *
 * @param RespData     The encoded data for sending via the network. Must be freed after usage by the procedure "FreeData()".
 * 
 * @return Length of the RespData if the encoding was success or "0" if not.
 */
size_t EncodeRespData(TestResult_s *TestResult, uint8_t **RespData);


/**
 * @brief
 * Decodes data for response to the parameter "**ResultData".
 *
 * @code
 * bool DecodeRespData(uint8_t Data[], size_t Len, TestResult_s *ResultData);
 * @code
 *
 * @param Data         The data for decoding.
 *
 * @param Len          The length of the data.
 * 
 * @param ResultData   The pointer in memory for decoded data. The memory must be already reserved.
 * 
 * @return             "true" if success or "false" if failed.
 */
bool DecodeRespData(uint8_t Data[], size_t Len, TestResult_s *ResultData);


/**
 * @brief
 * Frees the "**Data" allocated by by one of the procedures "DecodeReqData()", "DecodeReqData()" or "EncodeReqData()". 
 * No need to check the condition. It checks if the pointer is not NULL and only in this case it frees the memory.
 * After freeing the memory it sets the pointer to NULL.
 *
 * @code
 * void FreeData(uint8_t **Data);
 * @code
 *
 * @param Data  The pointer to pointer to the dynamically allocated memory for freeing. *
 */
void FreeData(uint8_t **Data);




/*======================================================================================================================*/

/**
 * @brief Set cursor to defined place in the screen. 
 * 
 * @param x Number of the column.
 * 
 * @param y Number of the row.
 */
void MoveCursor(int x, int y);


/**
 * @brief Moves cursor forward.
 * 
 * @param x Number steps to move.
 */
void MoveCursFw(int x);

/**
 * @brief Moves cursor Up.
 * 
 * @param y Number of steps to move.
 */
void MoveCursUp(int y);


/**
 * @brief Moves cursor backward.
 * 
 * @param x Number of steps to move.
 */
void MoveCursBw(int x);

/**
 * @brief Moves cursor down.
 * 
 * @param Number of steps to move.
 */
void MoveCursDn(int y);

/**
 * @brief Sets cursor place in the line were it exists.
 * 
 * @param col x-position in line where the cursor must be put.
 */
void MoveCursToCol(int col);

/**
 * @brief Clears line in console.
 * 
 * @param lp The type of cleaning: after cursor, before cursor or all the line from beginning to end.
 */
void ClearLine(LnPrt_e lp);


/**
 * @brief Prints horizontal scale bar in the cursor place
 * 
 * @param ScaleLength Total length of the scale including number. (A part of the length will be taken by the number.)
 * 
 * @param FilledLen   The part of the scale-bar that is filled.
 * 
 * @param Colors      The filled part of the bar and empty part of the bar colors.
 * 
 * @param Symbols     The symbols from which is built a filled and the empty parts
 * 
 * @param MaxValue    The biggest number shown in the scale.
 * 
 * @param Value       The value shown in the scale.
 * 
 * @param Units       The units that are shown in the scale right after value.
 */
void PrintHorizScale(uint32_t ScaleLength, uint32_t FilledLen, char *Colors[], char *Symbols[], uint32_t MaxValue, uint32_t Value, char Units[]);

/*======================================================================================================================*/

#endif  //  ____CommonData_h__

